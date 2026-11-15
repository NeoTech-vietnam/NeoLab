#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "HttpSrv.h"
#include "HttpSrv_Cfg.h"
#include "esp_http_server.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

struct FakeHttpMutex { int held; };

static httpd_uri_t routes[4];
static unsigned route_count;
static httpd_handle_t server_handle;
static TaskFunction_t queued_entry;
static void *queued_argument;
static httpd_req_t *async_request;
static uint64 now_us;
static unsigned mutex_count, stop_count, complete_count, async_count, delete_count;
static unsigned releases, frames, opened, streamed, delivered, unavailable_count, error_count;
static unsigned chunks, shutdown_count;
static unsigned busy_frames;
static int fail_mutex, fail_start, fail_route, fail_stop, fail_begin, fail_task, fail_send, fail_type;
static int fail_complete, chunk_failure = 2;
static int assert_shutdown_order;
static Neo_StatusType frame_result = NEO_OK;
static Neo_StatusType release_result = NEO_OK;
static Neo_StatusType text_result = NEO_OK;
static uint32 text_length = 2;
static char last_status[40], last_type[80];

void fake_http_log(const char *tag, const char *format, ...)
{
    assert(tag != NULL && format != NULL);
}

const char *esp_err_to_name(esp_err_t error)
{
    (void)error;
    return "fake native error";
}

uint64 LocalTime_GetUs(void) { return now_us; }

SemaphoreHandle_t xSemaphoreCreateMutex(void)
{
    if (fail_mutex) return NULL;
    SemaphoreHandle_t mutex = calloc(1, sizeof(*mutex));
    assert(mutex != NULL);
    ++mutex_count;
    return mutex;
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t wait)
{
    assert(mutex != NULL && mutex->held == 0 && wait == portMAX_DELAY);
    mutex->held = 1;
    return pdTRUE;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex)
{
    assert(mutex != NULL && mutex->held == 1);
    mutex->held = 0;
    return pdTRUE;
}

void vSemaphoreDelete(SemaphoreHandle_t mutex)
{
    assert(mutex != NULL && mutex->held == 0 && mutex_count > 0);
    --mutex_count;
    free(mutex);
}

BaseType_t xTaskCreate(TaskFunction_t entry, const char *name, uint32_t stack,
                       void *arg, UBaseType_t priority, TaskHandle_t *task)
{
    assert(entry != NULL && arg == async_request && name != NULL);
    assert(stack == HttpSrv_Config.worker_stack_bytes && priority > 0);
    if (fail_task) return pdFALSE;
    assert(queued_entry == NULL);
    queued_entry = entry;
    queued_argument = arg;
    *task = routes;
    return pdPASS;
}

void vTaskDelay(TickType_t ticks) { now_us += (uint64)ticks * 1000U; }
void vTaskDelete(TaskHandle_t task)
{
    assert(task == NULL);
    ++delete_count;
}

esp_err_t httpd_start(httpd_handle_t *handle, const httpd_config_t *config)
{
    assert(server_handle == NULL);
    assert(config->server_port == HttpSrv_Config.port);
    assert(config->send_wait_timeout == HttpSrv_Config.socket_timeout_s);
    assert(config->recv_wait_timeout == HttpSrv_Config.socket_timeout_s);
    route_count = 0;
    if (fail_start) return ESP_FAIL;
    server_handle = routes;
    *handle = server_handle;
    return ESP_OK;
}

esp_err_t httpd_stop(httpd_handle_t handle)
{
    assert(handle == server_handle && handle != NULL);
    assert(async_request == NULL && queued_entry == NULL);
    ++stop_count;
    if (fail_stop) return ESP_FAIL;
    server_handle = NULL;
    route_count = 0;
    return ESP_OK;
}

esp_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t *route)
{
    assert(handle == server_handle && route_count < 4);
    if (fail_route && route_count == 2) return ESP_FAIL;
    routes[route_count++] = *route;
    return ESP_OK;
}

esp_err_t httpd_resp_send_custom_err(httpd_req_t *req, const char *status, const char *text)
{
    assert(req != NULL && text != NULL);
    ++unavailable_count;
    snprintf(last_status, sizeof(last_status), "%s", status);
    return ESP_OK;
}

esp_err_t httpd_resp_send_500(httpd_req_t *req)
{
    assert(req != NULL);
    ++error_count;
    return ESP_OK;
}

esp_err_t httpd_resp_set_type(httpd_req_t *req, const char *type)
{
    assert(req != NULL);
    snprintf(last_type, sizeof(last_type), "%s", type);
    return fail_type ? ESP_FAIL : ESP_OK;
}

esp_err_t httpd_resp_set_hdr(httpd_req_t *req, const char *name, const char *value)
{
    assert(req != NULL && strcmp(name, "Content-Disposition") == 0);
    assert(strcmp(value, "inline; filename=capture.jpg") == 0);
    return ESP_OK;
}

esp_err_t httpd_resp_send(httpd_req_t *req, const char *text, ssize_t length)
{
    assert(req != NULL && text != NULL && length >= 0);
    return fail_send ? ESP_FAIL : ESP_OK;
}

esp_err_t httpd_resp_send_chunk(httpd_req_t *req, const char *text, ssize_t length)
{
    assert(req == async_request && text != NULL && length > 0);
    ++chunks;
    return chunks == (unsigned)chunk_failure ? ESP_FAIL : ESP_OK;
}

esp_err_t httpd_req_async_handler_begin(httpd_req_t *req, httpd_req_t **copy)
{
    if (fail_begin) return ESP_FAIL;
    assert(async_request == NULL);
    async_request = malloc(sizeof(*async_request));
    assert(async_request != NULL);
    *async_request = *req;
    async_request->asynchronous = 1;
    *copy = async_request;
    ++async_count;
    return ESP_OK;
}

esp_err_t httpd_req_async_handler_complete(httpd_req_t *req)
{
    assert(req == async_request && req->asynchronous);
    if (assert_shutdown_order) assert(shutdown_count == 1);
    free(async_request);
    async_request = NULL;
    ++complete_count;
    return fail_complete ? ESP_FAIL : ESP_OK;
}

int httpd_req_to_sockfd(httpd_req_t *req) { return req->fd; }

int shutdown(int fd, int how)
{
    assert(async_request != NULL && fd == async_request->fd && how == SHUT_RDWR);
    ++shutdown_count;
    return 0;
}

esp_err_t httpd_sess_trigger_close(httpd_handle_t handle, int fd)
{
    assert(handle == server_handle && fd == 123);
    return ESP_OK;
}

static Neo_StatusType make_text(char *text, uint32 capacity, uint32 *length)
{
    assert(capacity >= 2);
    memcpy(text, "{}", 2);
    *length = text_length;
    return text_result;
}

static Neo_StatusType make_frame(boolean capture, Neo_FrameType *frame)
{
    (void)capture;
    ++frames;
    if (busy_frames != 0) {
        --busy_frames;
        return NEO_E_BUSY;
    }
    if (frame_result != NEO_OK) return frame_result;
    static const uint8 jpeg[] = {0xff, 0xd8, 0xff, 0xd9};
    *frame = (Neo_FrameType){.data = jpeg, .length = sizeof(jpeg), .token = {0, 7}};
    return NEO_OK;
}

static Neo_StatusType release_frame(Neo_BufferTokenType token)
{
    assert(token.slot == 0 && token.generation == 7);
    ++releases;
    return release_result;
}

static void stream_opened(void) { ++opened; }
static void stream_frame(const Neo_FrameType *frame, boolean sent)
{
    assert(frame->length == 4);
    ++streamed;
    if (sent) ++delivered;
}

static const Neo_HttpPortsType ports = {
    .page = make_text, .metrics = make_text, .frame = make_frame, .release = release_frame,
    .stream_opened = stream_opened, .stream_frame = stream_frame,
};

static esp_err_t request(const char *uri)
{
    httpd_req_t req = {.handle = server_handle, .fd = 123};
    for (unsigned i = 0; i < route_count; ++i) {
        if (strcmp(routes[i].uri, uri) == 0) return routes[i].handler(&req);
    }
    assert(!"route not registered");
    return ESP_FAIL;
}

static void run_worker(void)
{
    assert(queued_entry != NULL);
    TaskFunction_t entry = queued_entry;
    void *argument = queued_argument;
    queued_entry = NULL;
    queued_argument = NULL;
    chunks = 0;
    shutdown_count = 0;
    assert_shutdown_order = 1;
    entry(argument);
    assert(async_request == NULL && shutdown_count == 1);
    assert_shutdown_order = 0;
}

int main(void)
{
    assert(HttpSrv_Stop() == NEO_OK);
    assert(HttpSrv_Start(NULL) == NEO_E_PARAM);
    Neo_HttpPortsType invalid = ports;
    invalid.release = NULL;
    assert(HttpSrv_Start(&invalid) == NEO_E_PARAM);
    fail_mutex = 1;
    assert(HttpSrv_Start(&ports) == NEO_E_MEMORY);
    fail_mutex = 0;
    fail_start = 1;
    assert(HttpSrv_Start(&ports) == NEO_E_IO);
    assert(mutex_count == 0);
    fail_start = 0;
    fail_route = 1;
    assert(HttpSrv_Start(&ports) == NEO_E_IO);
    assert(mutex_count == 0 && server_handle == NULL);
    fail_route = 0;
    assert(HttpSrv_Start(&ports) == NEO_OK);
    assert(route_count == 4 && HttpSrv_Start(&ports) == NEO_E_STATE);
    assert(request("/") == ESP_OK && strcmp(last_type, "text/html") == 0);
    assert(request("/metrics") == ESP_OK && strcmp(last_type, "application/json") == 0);
    text_result = NEO_E_IO;
    assert(request("/metrics") == ESP_OK && error_count == 1);
    text_result = NEO_OK;
    text_length = 2048;
    assert(request("/metrics") == ESP_OK && error_count == 2);
    text_length = 2;
    assert(request("/capture") == ESP_OK && releases == 1);
    fail_send = 1;
    assert(request("/capture") == ESP_FAIL && releases == 2);
    fail_send = 0;
    frame_result = NEO_E_BUSY;
    assert(request("/capture") == ESP_OK && releases == 2);
    assert(strcmp(last_status, "503 Service Unavailable") == 0);
    frame_result = NEO_E_IO;
    assert(request("/capture") == ESP_OK && error_count == 3 && releases == 2);
    frame_result = NEO_OK;
    release_result = NEO_E_IO;
    assert(request("/capture") == ESP_FAIL && releases == 3);
    release_result = NEO_OK;

    fail_begin = 1;
    assert(request("/stream") == ESP_FAIL && async_count == 0 && complete_count == 0);
    fail_begin = 0;
    fail_task = 1;
    assert(request("/stream") == ESP_OK && async_count == 1 && complete_count == 1);
    fail_task = 0;
    assert(request("/stream") == ESP_OK && queued_entry != NULL);
    unsigned unavailable_before = unavailable_count;
    assert(request("/stream") == ESP_OK && unavailable_count == unavailable_before + 1);
    assert(strcmp(last_status, "503 Service Unavailable") == 0);
    /* The actual stream work is queued: normal routes can execute meanwhile. */
    assert(request("/metrics") == ESP_OK);
    assert(request("/capture") == ESP_OK && releases == 4);
    run_worker();
    assert(opened == 1 && streamed == 1 && delivered == 0 && releases == 5);
    assert(complete_count == 2 && delete_count == 1);

    /* One successful frame, then a send failure; both leases are released. */
    chunk_failure = 4;
    assert(request("/stream") == ESP_OK);
    run_worker();
    assert(streamed == 3 && delivered == 1 && releases == 7);
    assert(complete_count == 3 && delete_count == 2);
    frame_result = NEO_E_IO;
    assert(request("/stream") == ESP_OK);
    run_worker();
    assert(releases == 7 && complete_count == 4);
    frame_result = NEO_OK;
    fail_type = 1;
    assert(request("/stream") == ESP_OK);
    run_worker();
    assert(releases == 7 && complete_count == 5);
    fail_type = 0;
    fail_complete = 1;
    assert(request("/stream") == ESP_OK);
    run_worker();
    assert(complete_count == 6 && async_request == NULL);
    fail_complete = 0;

    /* Temporary pool exhaustion yields, then retries without leaking a lease. */
    busy_frames = 1;
    chunk_failure = 2;
    unsigned released_before = releases;
    uint64 before_wait = now_us;
    assert(request("/stream") == ESP_OK);
    run_worker();
    assert(busy_frames == 0 && now_us > before_wait && releases == released_before + 1);
    /* A release error still completes the asynchronous request exactly once. */
    release_result = NEO_E_IO;
    assert(request("/stream") == ESP_OK);
    run_worker();
    assert(releases == released_before + 2);
    release_result = NEO_OK;

    assert(request("/stream") == ESP_OK);
    unsigned stops_before = stop_count;
    assert(HttpSrv_Stop() == NEO_E_TIMEOUT);
    assert(stop_count == stops_before && mutex_count == 1 && server_handle != NULL);
    unavailable_before = unavailable_count;
    assert(request("/metrics") == ESP_OK && unavailable_count == unavailable_before + 1);
    run_worker();
    fail_stop = 1;
    assert(HttpSrv_Stop() == NEO_E_IO);
    assert(mutex_count == 1 && server_handle != NULL);
    fail_stop = 0;
    assert(HttpSrv_Stop() == NEO_OK && mutex_count == 0);
    assert(HttpSrv_Stop() == NEO_OK);
    assert(HttpSrv_Start(&ports) == NEO_OK);
    assert(request("/capture") == ESP_OK);
    assert(HttpSrv_Stop() == NEO_OK);
    assert(async_count == complete_count);
    puts("HTTP transport lifecycle tests passed");
    return 0;
}
