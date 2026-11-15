#include "HttpSrv_Internal.h"
#include "HttpSrv_Cfg.h"
#include "LocalTime.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lwip/sockets.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static httpd_handle_t server;
static SemaphoreHandle_t state_lock;
static Neo_HttpPortsType app;
static boolean stopping;
static boolean stream_active;

static void lock(void) { (void)xSemaphoreTake(state_lock, portMAX_DELAY); }
static void unlock(void) { (void)xSemaphoreGive(state_lock); }

static boolean is_stopping(void)
{
    lock();
    boolean result = stopping;
    unlock();
    return result;
}

static esp_err_t unavailable(httpd_req_t *req, const char *message)
{
    return httpd_resp_send_custom_err(req, "503 Service Unavailable", message);
}

static esp_err_t text_response(httpd_req_t *req, boolean page)
{
    if (is_stopping()) return unavailable(req, "Service stopping");
    char text[2048];
    uint32 length = 0;
    Neo_StatusType result = page ? app.page(text, sizeof(text), &length)
                                : app.metrics(text, sizeof(text), &length);
    if (result != NEO_OK || length >= sizeof(text)) return httpd_resp_send_500(req);
    httpd_resp_set_type(req, page ? "text/html" : "application/json");
    return httpd_resp_send(req, text, (ssize_t)length);
}

static esp_err_t page_handler(httpd_req_t *req) { return text_response(req, TRUE); }
static esp_err_t metrics_handler(httpd_req_t *req) { return text_response(req, FALSE); }

static esp_err_t capture_handler(httpd_req_t *req)
{
    if (is_stopping()) return unavailable(req, "Service stopping");
    Neo_FrameType frame;
    Neo_StatusType result = app.frame(TRUE, &frame);
    if (result == NEO_E_BUSY) return unavailable(req, "JPEG buffers busy");
    if (result != NEO_OK) return httpd_resp_send_500(req);
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    esp_err_t sent = httpd_resp_send(req, (const char *)frame.data, (ssize_t)frame.length);
    Neo_StatusType released = app.release(frame.token);
    if (released != NEO_OK) {
        ESP_LOGE("HttpSrv", "Capture buffer release failed: %d", released);
        return ESP_FAIL;
    }
    return sent;
}

static void stream_worker(void *argument)
{
    httpd_req_t *req = argument;
    /* The server/session remain alive until this worker acknowledges completion. */
    int fd = httpd_req_to_sockfd(req);
    esp_err_t sent = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
    app.stream_opened();
    while (sent == ESP_OK && !is_stopping()) {
        Neo_FrameType frame;
        Neo_StatusType result = app.frame(FALSE, &frame);
        if (result == NEO_E_BUSY) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        if (result != NEO_OK) break;
        char header[96];
        int length = snprintf(header, sizeof(header),
            "\r\n--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %" PRIu32 "\r\n\r\n", frame.length);
        if (length < 0 || length >= (int)sizeof(header)) sent = ESP_FAIL;
        else {
            sent = httpd_resp_send_chunk(req, header, length);
            if (sent == ESP_OK) sent = httpd_resp_send_chunk(req, (const char *)frame.data, frame.length);
        }
        app.stream_frame(&frame, sent == ESP_OK);
        result = app.release(frame.token);
        if (result != NEO_OK) {
            ESP_LOGE("HttpSrv", "Stream buffer release failed: %d", result);
            break;
        }
    }
    /* Shut down while the async lease still protects the socket/session. */
    (void)shutdown(fd, SHUT_RDWR);
    /* Completion frees req even when its control-socket notification fails. */
    esp_err_t completed = httpd_req_async_handler_complete(req);
    if (completed != ESP_OK) ESP_LOGW("HttpSrv", "Async completion notification failed");
    lock();
    stream_active = FALSE;
    unlock();
    /* No application, server or module state may be accessed after this ack. */
    vTaskDelete(NULL);
}

static esp_err_t stream_handler(httpd_req_t *req)
{
    lock();
    if (stopping || stream_active) {
        unlock();
        return unavailable(req, "Only one stream is supported");
    }
    stream_active = TRUE; /* Reserve before an asynchronous request can be created. */
    unlock();
    httpd_req_t *async = NULL;
    esp_err_t result = httpd_req_async_handler_begin(req, &async);
    if (result == ESP_OK) {
        TaskHandle_t worker = NULL;
        if (xTaskCreate(stream_worker, "NeoPreview", HttpSrv_Config.worker_stack_bytes,
                        async, 4, &worker) == pdPASS) return ESP_OK;
        (void)unavailable(async, "Cannot create stream worker");
        (void)httpd_req_async_handler_complete(async);
    } else {
        (void)unavailable(req, "Cannot create asynchronous request");
    }
    lock();
    stream_active = FALSE;
    unlock();
    return result == ESP_OK ? ESP_OK : ESP_FAIL;
}

Neo_StatusType HttpSrv_BackendStop(void)
{
    if (state_lock == NULL) return NEO_OK;
    lock();
    stopping = TRUE;
    unlock();
    uint64 deadline = LocalTime_GetUs() + (uint64)HttpSrv_Config.stop_timeout_ms * 1000U;
    for (;;) {
        lock();
        boolean busy = stream_active;
        unlock();
        if (!busy) break;
        if (LocalTime_GetUs() >= deadline) return NEO_E_TIMEOUT;
        vTaskDelay(pdMS_TO_TICKS(10));
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
        HttpSrv_Config.worker_stack_bytes < 4096U || HttpSrv_Config.stop_timeout_ms == 0U) return NEO_E_PARAM;
    state_lock = xSemaphoreCreateMutex();
    if (state_lock == NULL) return NEO_E_MEMORY;
    app = *ports;
    stopping = FALSE;
    stream_active = FALSE;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = HttpSrv_Config.port;
    config.send_wait_timeout = HttpSrv_Config.socket_timeout_s;
    config.recv_wait_timeout = HttpSrv_Config.socket_timeout_s;
    config.stack_size = 6144;
    esp_err_t result = httpd_start(&server, &config);
    const httpd_uri_t routes[] = {
        {.uri = "/", .method = HTTP_GET, .handler = page_handler},
        {.uri = "/capture", .method = HTTP_GET, .handler = capture_handler},
        {.uri = "/metrics", .method = HTTP_GET, .handler = metrics_handler},
        {.uri = "/stream", .method = HTTP_GET, .handler = stream_handler}
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
