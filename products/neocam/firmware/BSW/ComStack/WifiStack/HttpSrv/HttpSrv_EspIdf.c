#include "HttpSrv_Internal.h"
#include "HttpSrv_Cfg.h"
#include "LocalTime.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lwip/sockets.h"
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define PAGE_CAPACITY 6144U
#define PROFILE_REQUEST_CAPACITY 128U

typedef struct {
    boolean active;
    boolean photo;
    httpd_req_t *request;
    int fd;
    uint64 send_deadline_us;
} HttpWorker;

static httpd_handle_t server;
static SemaphoreHandle_t state_lock;
static Neo_HttpPortsType app;
static boolean stopping;
/* The fixed slots bound both asynchronous requests and retained image leases. */
static HttpWorker stream_worker_state;
static HttpWorker image_worker_state;

static void lock(void) { (void)xSemaphoreTake(state_lock, portMAX_DELAY); }
static void unlock(void) { (void)xSemaphoreGive(state_lock); }

static boolean is_stopping(void)
{
    lock();
    boolean result = stopping;
    unlock();
    return result;
}

static uint64 deadline_after(uint32 milliseconds)
{
    uint64 now = LocalTime_GetUs();
    uint64 duration = (uint64)milliseconds * 1000U;
    return now > UINT64_MAX - duration ? UINT64_MAX : now + duration;
}

static uint64 add_period(uint64 time, uint64 period)
{
    return time > UINT64_MAX - period ? UINT64_MAX : time + period;
}

/* The SDK may call this repeatedly for headers/chunks/partial writes. Every call
 * uses the same frame deadline, so intermittent progress cannot extend it. */
static int deadline_send(httpd_handle_t handle, int fd, const char *data, size_t length, int flags)
{
    lock();
    HttpWorker *worker = NULL;
    if (handle == server) {
        if (stream_worker_state.active && stream_worker_state.fd == fd) worker = &stream_worker_state;
        else if (image_worker_state.active && image_worker_state.fd == fd) worker = &image_worker_state;
    }
    unlock();
    if (worker == NULL || fd < 0 || fd >= FD_SETSIZE) return HTTPD_SOCK_ERR_INVALID;
    while (!is_stopping()) {
        uint64 now = LocalTime_GetUs();
        if (now >= worker->send_deadline_us) return HTTPD_SOCK_ERR_TIMEOUT;
        size_t amount = length > (size_t)INT_MAX ? (size_t)INT_MAX : length;
        int sent = send(fd, data, amount, flags | MSG_DONTWAIT);
        if (sent > 0) return sent;
        if (sent == 0) return HTTPD_SOCK_ERR_FAIL;
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) return HTTPD_SOCK_ERR_FAIL;
        now = LocalTime_GetUs();
        if (now >= worker->send_deadline_us) return HTTPD_SOCK_ERR_TIMEOUT;
        uint64 remaining = worker->send_deadline_us - now;
        /* Short waits also observe module stop without changing the socket's
         * flags or the other HTTP sessions' normal two-second timeouts. */
        if (remaining > 50000U) remaining = 50000U;
        struct timeval timeout = {.tv_sec = 0, .tv_usec = (long)remaining};
        fd_set writable;
        FD_ZERO(&writable);
        FD_SET(fd, &writable);
        int ready = select(fd + 1, NULL, &writable, NULL, &timeout);
        if (ready < 0 && errno != EINTR) return HTTPD_SOCK_ERR_FAIL;
    }
    return HTTPD_SOCK_ERR_FAIL;
}

static esp_err_t unavailable(httpd_req_t *req, const char *message)
{
    return httpd_resp_send_custom_err(req, "503 Service Unavailable", message);
}

static esp_err_t text_response(httpd_req_t *req, unsigned kind)
{
    if (is_stopping()) return unavailable(req, "Service stopping");
    char text[PAGE_CAPACITY];
    uint32 length = 0;
    Neo_StatusType result = kind == 0 ? app.page(text, sizeof(text), &length)
                          : kind == 1 ? app.metrics(text, sizeof(text), &length)
                                      : app.profile(text, sizeof(text), &length);
    if (result != NEO_OK || length >= sizeof(text)) return httpd_resp_send_500(req);
    if (httpd_resp_set_type(req, kind == 0 ? "text/html" : "application/json") != ESP_OK) return ESP_FAIL;
    return httpd_resp_send(req, text, (ssize_t)length);
}

static esp_err_t page_handler(httpd_req_t *req) { return text_response(req, 0); }
static esp_err_t metrics_handler(httpd_req_t *req) { return text_response(req, 1); }
static esp_err_t profile_handler(httpd_req_t *req) { return text_response(req, 2); }

static const char *skip_space(const char *position, const char *end)
{
    while (position < end && (*position == ' ' || *position == '\t' || *position == '\r' || *position == '\n')) ++position;
    return position;
}

/* Accept the one documented command object, not arbitrary camera registers. */
static boolean parse_profile(const char *text, size_t length, char name[32])
{
    const char *end = text + length;
    const char *at = skip_space(text, end);
    if (at == end || *at++ != '{') return FALSE;
    at = skip_space(at, end);
    static const char key[] = "\"profile\"";
    if ((size_t)(end - at) < sizeof(key) - 1 || memcmp(at, key, sizeof(key) - 1) != 0) return FALSE;
    at = skip_space(at + sizeof(key) - 1, end);
    if (at == end || *at++ != ':') return FALSE;
    at = skip_space(at, end);
    if (at == end || *at++ != '"') return FALSE;
    size_t used = 0;
    while (at < end && *at != '"') {
        char value = *at++;
        if (!((value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
              (value >= '0' && value <= '9') || value == '-' || value == '_') || used >= 31) return FALSE;
        name[used++] = value;
    }
    if (used == 0 || at == end) return FALSE;
    name[used] = '\0';
    at = skip_space(at + 1, end);
    if (at == end || *at++ != '}') return FALSE;
    return skip_space(at, end) == end;
}

static esp_err_t select_profile_handler(httpd_req_t *req)
{
    if (is_stopping()) return unavailable(req, "Service stopping");
    if (req->content_len == 0 || req->content_len > PROFILE_REQUEST_CAPACITY)
        return httpd_resp_send_custom_err(req, "400 Bad Request", "Expected a small profile command");
    char text[PROFILE_REQUEST_CAPACITY];
    size_t received = 0;
    while (received < req->content_len) {
        int result = httpd_req_recv(req, text + received, req->content_len - received);
        if (result <= 0) return httpd_resp_send_custom_err(req, "400 Bad Request", "Incomplete profile command");
        received += (size_t)result;
    }
    char name[32];
    if (!parse_profile(text, received, name))
        return httpd_resp_send_custom_err(req, "400 Bad Request", "Expected {\"profile\":\"name\"}");
    Neo_StatusType result = app.select_profile(name);
    if (result == NEO_E_BUSY) return httpd_resp_send_custom_err(req, "409 Conflict", "Profile operation busy");
    if (result == NEO_E_PARAM) return httpd_resp_send_custom_err(req, "400 Bad Request", "Unknown profile");
    if (result != NEO_OK) return httpd_resp_send_500(req);
    if (httpd_resp_set_status(req, "202 Accepted") != ESP_OK ||
        httpd_resp_set_type(req, "application/json") != ESP_OK) return ESP_FAIL;
    return httpd_resp_send(req, "{\"accepted\":true}", 17);
}

static boolean release_frame(Neo_BufferTokenType token)
{
    Neo_StatusType released = app.release(token);
    if (released != NEO_OK) ESP_LOGE("HttpSrv", "Image buffer release failed: %d", released);
    return released == NEO_OK;
}

static void finish_worker(HttpWorker *worker)
{
    /* Close before completion: a partially sent MIME part/JPEG cannot be reused
     * as a keepalive response, and the async lease still protects the session. */
    (void)shutdown(worker->fd, SHUT_RDWR);
    if (httpd_req_async_handler_complete(worker->request) != ESP_OK)
        ESP_LOGW("HttpSrv", "Async completion notification failed");
    lock();
    worker->request = NULL;
    worker->active = FALSE;
    unlock();
    /* Acknowledge only after every callback/session access has finished. */
    vTaskDelete(NULL);
}

static void image_worker(void *argument)
{
    HttpWorker *worker = argument;
    httpd_req_t *req = worker->request;
    worker->send_deadline_us = deadline_after(HttpSrv_Config.image_send_timeout_ms);
    if (httpd_sess_set_send_override(req->handle, worker->fd, deadline_send) != ESP_OK) {
        finish_worker(worker);
        return;
    }
    if (!is_stopping()) {
        Neo_FrameType frame;
        Neo_StatusType result = worker->photo ? app.photo(&frame) : app.frame(TRUE, &frame);
        /* Photo acquisition restores preview configuration before returning. */
        worker->send_deadline_us = deadline_after(HttpSrv_Config.image_send_timeout_ms);
        if (result == NEO_OK) {
            if (httpd_resp_set_type(req, "image/jpeg") == ESP_OK &&
                httpd_resp_set_hdr(req, "Content-Disposition", worker->photo
                    ? "inline; filename=photo.jpg" : "inline; filename=capture.jpg") == ESP_OK)
                (void)httpd_resp_send(req, (const char *)frame.data, (ssize_t)frame.length);
            (void)release_frame(frame.token);
        } else if (result == NEO_E_BUSY || result == NEO_E_NOT_READY) {
            (void)unavailable(req, "Image acquisition busy");
        } else {
            (void)httpd_resp_send_500(req);
        }
    }
    finish_worker(worker);
}

static void stream_worker(void *argument)
{
    HttpWorker *worker = argument;
    httpd_req_t *req = worker->request;
    worker->send_deadline_us = deadline_after(HttpSrv_Config.stream_send_timeout_ms);
    esp_err_t sent = httpd_sess_set_send_override(req->handle, worker->fd, deadline_send);
    if (sent == ESP_OK) sent = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
    uint64 sequence = 0;
    uint64 next_frame_us = 0;
    uint32 pacing_epoch = 0;
    uint16 pacing_fps = 0;
    if (sent == ESP_OK) app.stream_opened();
    while (sent == ESP_OK && !is_stopping()) {
        uint64 now = LocalTime_GetUs();
        if (now < next_frame_us) {
            uint64 wait_ms = (next_frame_us - now + 999U) / 1000U;
            if (wait_ms > 20U) wait_ms = 20U;
            TickType_t ticks = pdMS_TO_TICKS((uint32)wait_ms);
            vTaskDelay(ticks != 0 ? ticks : 1);
            continue;
        }
        Neo_FrameType frame;
        Neo_StatusType result = app.stream_next(sequence, 100, &frame);
        if (result == NEO_E_BUSY || result == NEO_E_NOT_READY || result == NEO_E_TIMEOUT) {
            TickType_t ticks = pdMS_TO_TICKS(10);
            vTaskDelay(ticks != 0 ? ticks : 1);
            continue;
        }
        if (result != NEO_OK) break;
        if (frame.sequence <= sequence) {
            if (!release_frame(frame.token)) break;
            TickType_t ticks = pdMS_TO_TICKS(10);
            vTaskDelay(ticks != 0 ? ticks : 1);
            continue;
        }
        sequence = frame.sequence;
        char header[256];
        int length = snprintf(header, sizeof(header),
            "\r\n--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %" PRIu32
            "\r\nX-Sequence: %" PRIu64 "\r\nX-Capture-Time-Us: %" PRIu64
            "\r\nX-Profile-Epoch: %" PRIu32 "\r\n\r\n",
            frame.length, frame.sequence, frame.timestamp_us, frame.profile_epoch);
        uint64 started = LocalTime_GetUs();
        worker->send_deadline_us = deadline_after(HttpSrv_Config.stream_send_timeout_ms);
        if (length < 0 || length >= (int)sizeof(header)) sent = ESP_FAIL;
        else {
            sent = httpd_resp_send_chunk(req, header, length);
            if (sent == ESP_OK) sent = httpd_resp_send_chunk(req, (const char *)frame.data, (ssize_t)frame.length);
        }
        uint64 finished = LocalTime_GetUs();
        uint64 duration = finished - started;
        app.stream_frame(&frame, sent == ESP_OK, duration > UINT32_MAX ? UINT32_MAX : (uint32)duration);
        uint64 period = frame.target_fps != 0 ? 1000000U / frame.target_fps : 0;
        if (period == 0) next_frame_us = 0;
        else {
            /* Preserve the phase across RTOS wakeup rounding: at 100 Hz,
             * 30 FPS needs alternating 30/40 ms waits rather than always 40.
             * Changed profiles and a missed full period establish a new phase. */
            if (next_frame_us == 0 || pacing_epoch != frame.profile_epoch ||
                pacing_fps != frame.target_fps ||
                (started > next_frame_us && started - next_frame_us > period))
                next_frame_us = started;
            next_frame_us = add_period(next_frame_us, period);
            /* Never catch up by sending a burst after a slow network write.
             * Skip one missed opportunity, or rebase after a larger stall. */
            if (finished >= next_frame_us) {
                if (finished - next_frame_us > period) next_frame_us = add_period(finished, period);
                else next_frame_us = add_period(next_frame_us, period);
                if (finished >= next_frame_us) next_frame_us = add_period(finished, period);
            }
        }
        pacing_epoch = frame.profile_epoch;
        pacing_fps = frame.target_fps;
        if (!release_frame(frame.token)) break;
    }
    finish_worker(worker);
}

static esp_err_t start_worker(httpd_req_t *req, HttpWorker *worker, boolean photo)
{
    lock();
    if (stopping || worker->active) {
        unlock();
        return unavailable(req, "Image request already active");
    }
    worker->active = TRUE;
    worker->photo = photo;
    worker->fd = httpd_req_to_sockfd(req);
    unlock();
    httpd_req_t *async = NULL;
    esp_err_t result = httpd_req_async_handler_begin(req, &async);
    if (result == ESP_OK) {
        worker->request = async;
        TaskHandle_t task = NULL;
        TaskFunction_t entry = worker == &stream_worker_state ? stream_worker : image_worker;
        if (xTaskCreate(entry, worker == &stream_worker_state ? "NeoPreview" : "NeoImage",
                        HttpSrv_Config.worker_stack_bytes, worker, 4, &task) == pdPASS) return ESP_OK;
        (void)unavailable(async, "Cannot create image worker");
        (void)shutdown(worker->fd, SHUT_RDWR);
        (void)httpd_req_async_handler_complete(async);
    } else {
        (void)unavailable(req, "Cannot create asynchronous request");
    }
    lock();
    worker->request = NULL;
    worker->active = FALSE;
    unlock();
    return result == ESP_OK ? ESP_OK : ESP_FAIL;
}

static esp_err_t stream_handler(httpd_req_t *req) { return start_worker(req, &stream_worker_state, FALSE); }
static esp_err_t capture_handler(httpd_req_t *req) { return start_worker(req, &image_worker_state, FALSE); }
static esp_err_t photo_handler(httpd_req_t *req) { return start_worker(req, &image_worker_state, TRUE); }

Neo_StatusType HttpSrv_BackendStop(void)
{
    if (state_lock == NULL) return NEO_OK;
    lock();
    stopping = TRUE;
    unlock();
    uint64 deadline = deadline_after(HttpSrv_Config.stop_timeout_ms);
    for (;;) {
        lock();
        boolean busy = stream_worker_state.active || image_worker_state.active;
        unlock();
        if (!busy) break;
        if (LocalTime_GetUs() >= deadline) return NEO_E_TIMEOUT;
        TickType_t ticks = pdMS_TO_TICKS(10);
        vTaskDelay(ticks != 0 ? ticks : 1);
    }
    if (server != NULL) {
        esp_err_t result = httpd_stop(server);
        if (result != ESP_OK) {
            ESP_LOGE("HttpSrv", "HTTP stop failed: %s", esp_err_to_name(result));
            return NEO_E_IO;
        }
        server = NULL;
    }
    vSemaphoreDelete(state_lock);
    state_lock = NULL;
    memset(&app, 0, sizeof(app));
    return NEO_OK;
}

Neo_StatusType HttpSrv_BackendStart(const Neo_HttpPortsType *ports)
{
    if (state_lock != NULL) return NEO_E_STATE;
    if (HttpSrv_Config.port == 0U || HttpSrv_Config.socket_timeout_s == 0U ||
        HttpSrv_Config.worker_stack_bytes < 4096U || HttpSrv_Config.stop_timeout_ms == 0U ||
        HttpSrv_Config.stream_send_timeout_ms == 0U || HttpSrv_Config.image_send_timeout_ms == 0U) return NEO_E_PARAM;
    state_lock = xSemaphoreCreateMutex();
    if (state_lock == NULL) return NEO_E_MEMORY;
    app = *ports;
    stopping = FALSE;
    memset(&stream_worker_state, 0, sizeof(stream_worker_state));
    memset(&image_worker_state, 0, sizeof(image_worker_state));
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = HttpSrv_Config.port;
    config.send_wait_timeout = HttpSrv_Config.socket_timeout_s;
    config.recv_wait_timeout = HttpSrv_Config.socket_timeout_s;
    config.stack_size = 12288;
    esp_err_t result = httpd_start(&server, &config);
    const httpd_uri_t routes[] = {
        {.uri = "/", .method = HTTP_GET, .handler = page_handler},
        {.uri = "/capture", .method = HTTP_GET, .handler = capture_handler},
        {.uri = "/metrics", .method = HTTP_GET, .handler = metrics_handler},
        {.uri = "/stream", .method = HTTP_GET, .handler = stream_handler},
        {.uri = "/photo", .method = HTTP_GET, .handler = photo_handler},
        {.uri = "/api/profile", .method = HTTP_GET, .handler = profile_handler},
        {.uri = "/api/profile", .method = HTTP_POST, .handler = select_profile_handler}
    };
    for (uint32 i = 0; result == ESP_OK && i < sizeof(routes) / sizeof(routes[0]); ++i)
        result = httpd_register_uri_handler(server, &routes[i]);
    if (result != ESP_OK) {
        ESP_LOGE("HttpSrv", "HTTP initialization failed: %s", esp_err_to_name(result));
        Neo_StatusType cleanup = HttpSrv_BackendStop();
        if (cleanup != NEO_OK) ESP_LOGE("HttpSrv", "HTTP rollback failed: %d", cleanup);
        return NEO_E_IO;
    }
    return NEO_OK;
}
