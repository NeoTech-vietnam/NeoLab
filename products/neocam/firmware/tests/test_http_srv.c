#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "HttpSrv.h"
#include "HttpSrv_Cfg.h"
#include "esp_http_server.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

struct FakeHttpMutex { int held; };
typedef struct { httpd_req_t *req; httpd_send_func_t sender; unsigned shutdowns; } Session;
typedef struct { TaskFunction_t entry; void *arg; } Task;
static httpd_uri_t routes[7];
static Session sessions[2];
static Task tasks[2];
static httpd_handle_t server;
static unsigned route_count, next_fd = 100, mutex_count, stops, completes, asyncs, deletes;
static unsigned releases, frames, photos, streamed, delivered, errors, selects, send_calls, profile_calls;
static unsigned busy_frames, duplicates, override_failures, completion_failures;
static int fail_mutex, fail_start, fail_route, fail_stop, fail_begin, fail_task, fail_type, fail_recv;
static int blocked, hard_error, partial;
static uint64 now_us, sequence = 1, last_after;
static uint32 send_step_us, reported_send_us, text_length = 2;
static uint16 target_fps;
uint32_t fake_http_tick_ms = 1;
static uint64 frame_limit = 3;
static uint64 send_starts[1024], sent_sequences[1024];
static uint32 sent_epochs[1024];
static unsigned pacing_samples;
static boolean measure_pacing;
static uint64 epoch_change_sequence, slow_send_sequence;
static uint32 changed_epoch = 8;
static uint16 changed_fps;
static uint32 delayed_frame_us;
static Neo_StatusType frame_result = NEO_OK, release_result = NEO_OK, text_result = NEO_OK, selection_result = NEO_OK;
static char last_status[40], last_type[80], last_profile[32], wire[8192];
static size_t wire_length;

void fake_http_log(const char *tag, const char *format, ...) { assert(tag && format); }
const char *esp_err_to_name(esp_err_t error) { (void)error; return "fake error"; }
uint64 LocalTime_GetUs(void) { return now_us; }
SemaphoreHandle_t xSemaphoreCreateMutex(void)
{
    if (fail_mutex) return NULL;
    SemaphoreHandle_t mutex = calloc(1, sizeof(*mutex));
    assert(mutex); ++mutex_count; return mutex;
}
BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t wait)
{ assert(mutex && !mutex->held && wait == portMAX_DELAY); mutex->held = 1; return pdTRUE; }
BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex)
{ assert(mutex && mutex->held); mutex->held = 0; return pdTRUE; }
void vSemaphoreDelete(SemaphoreHandle_t mutex)
{ assert(mutex && !mutex->held && mutex_count); --mutex_count; free(mutex); }
BaseType_t xTaskCreate(TaskFunction_t entry, const char *name, uint32_t stack, void *arg, UBaseType_t priority, TaskHandle_t *handle)
{
    assert(entry && name && arg && priority && stack == HttpSrv_Config.worker_stack_bytes);
    if (fail_task) return pdFALSE;
    for (unsigned i = 0; i < 2; ++i) if (!tasks[i].entry) {
        tasks[i] = (Task){entry, arg}; *handle = &tasks[i]; return pdPASS;
    }
    assert(!"More than two workers"); return pdFALSE;
}
void vTaskDelay(TickType_t ticks) { assert(ticks); now_us += (uint64)ticks * fake_http_tick_ms * 1000U; }
void vTaskDelete(TaskHandle_t task) { assert(!task); ++deletes; }
esp_err_t httpd_start(httpd_handle_t *handle, const httpd_config_t *cfg)
{
    assert(!server && cfg->server_port == HttpSrv_Config.port && cfg->stack_size >= 12288);
    assert(cfg->send_wait_timeout == 2 && cfg->recv_wait_timeout == 2);
    route_count = 0; if (fail_start) return ESP_FAIL;
    *handle = server = routes; return ESP_OK;
}
esp_err_t httpd_stop(httpd_handle_t handle)
{
    assert(handle && handle == server);
    for (unsigned i = 0; i < 2; ++i) assert(!sessions[i].req && !tasks[i].entry);
    ++stops; if (fail_stop) return ESP_FAIL;
    server = NULL; route_count = 0; return ESP_OK;
}
esp_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t *route)
{
    assert(handle == server && route_count < 7);
    if (fail_route && route_count == 2) return ESP_FAIL;
    routes[route_count++] = *route; return ESP_OK;
}
static Session *session_for(int fd)
{
    for (unsigned i = 0; i < 2; ++i) if (sessions[i].req && sessions[i].req->fd == fd) return &sessions[i];
    return NULL;
}
esp_err_t httpd_sess_set_send_override(httpd_handle_t handle, int fd, httpd_send_func_t sender)
{
    Session *session = session_for(fd); assert(handle == server && session && sender);
    if (override_failures) { --override_failures; return ESP_FAIL; }
    session->sender = sender; return ESP_OK;
}
ssize_t fake_socket_send(int fd, const void *data, size_t length, int flags)
{
    assert(session_for(fd) && data && (flags & MSG_DONTWAIT)); ++send_calls;
    if (hard_error) { errno = EPIPE; return -1; }
    if (blocked) { errno = EAGAIN; return -1; }
    now_us += send_step_us;
    return (ssize_t)(partial && length > 3 ? 3 : length);
}
int fake_socket_select(int limit, fd_set *readable, fd_set *writable, fd_set *failed, struct timeval *timeout)
{
    assert(limit > 0 && !readable && writable && !failed && timeout); ++selects;
    now_us += (uint64)timeout->tv_sec * 1000000U + (uint64)timeout->tv_usec;
    return blocked ? 0 : 1;
}
static esp_err_t send_text(httpd_req_t *req, const char *text, size_t length)
{
    assert(req && text);
    if (wire_length + length < sizeof(wire)) {
        memcpy(wire + wire_length, text, length); wire_length += length; wire[wire_length] = '\0';
    }
    Session *session = session_for(req->fd);
    if (session && session->sender) while (length) {
        int sent = session->sender(req->handle, req->fd, text, length, 0);
        if (sent <= 0) return ESP_FAIL;
        assert((size_t)sent <= length); text += sent; length -= (size_t)sent;
    }
    return ESP_OK;
}
esp_err_t httpd_resp_send_custom_err(httpd_req_t *req, const char *status, const char *text)
{ snprintf(last_status, sizeof(last_status), "%s", status); return send_text(req, text, strlen(text)); }
esp_err_t httpd_resp_send_500(httpd_req_t *req)
{ ++errors; return httpd_resp_send_custom_err(req, "500 Internal Server Error", "Error"); }
esp_err_t httpd_resp_set_type(httpd_req_t *req, const char *type)
{ assert(req); snprintf(last_type, sizeof(last_type), "%s", type); return fail_type ? ESP_FAIL : ESP_OK; }
esp_err_t httpd_resp_set_status(httpd_req_t *req, const char *status)
{ assert(req); snprintf(last_status, sizeof(last_status), "%s", status); return ESP_OK; }
esp_err_t httpd_resp_set_hdr(httpd_req_t *req, const char *name, const char *value)
{
    assert(req && strcmp(name, "Content-Disposition") == 0);
    assert(strcmp(value, "inline; filename=capture.jpg") == 0 || strcmp(value, "inline; filename=photo.jpg") == 0);
    return ESP_OK;
}
esp_err_t httpd_resp_send(httpd_req_t *req, const char *text, ssize_t length)
{ assert(length >= 0); return send_text(req, text, (size_t)length); }
esp_err_t httpd_resp_send_chunk(httpd_req_t *req, const char *text, ssize_t length)
{ assert(req->asynchronous && length > 0); return send_text(req, text, (size_t)length); }
int httpd_req_recv(httpd_req_t *req, char *buffer, size_t length)
{
    if (fail_recv) return -1;
    assert(req->body && length <= req->content_len - req->body_read);
    if (length > 3) length = 3; /* Exercise fragmented request bodies. */
    memcpy(buffer, req->body + req->body_read, length); req->body_read += length; return (int)length;
}
esp_err_t httpd_req_async_handler_begin(httpd_req_t *req, httpd_req_t **copy)
{
    if (fail_begin) return ESP_FAIL;
    for (unsigned i = 0; i < 2; ++i) if (!sessions[i].req) {
        sessions[i] = (Session){.req = malloc(sizeof(**copy))}; assert(sessions[i].req);
        *sessions[i].req = *req; sessions[i].req->asynchronous = 1; *copy = sessions[i].req; ++asyncs; return ESP_OK;
    }
    assert(!"More than two async requests"); return ESP_FAIL;
}
esp_err_t httpd_req_async_handler_complete(httpd_req_t *req)
{
    Session *session = session_for(req->fd);
    assert(session && req == session->req && session->shutdowns == 1);
    free(session->req); *session = (Session){0}; ++completes;
    if (completion_failures) { --completion_failures; return ESP_FAIL; }
    return ESP_OK;
}
int httpd_req_to_sockfd(httpd_req_t *req) { return req->fd; }
int shutdown(int fd, int how)
{ Session *s = session_for(fd); assert(s && how == SHUT_RDWR && !s->shutdowns); ++s->shutdowns; return 0; }
esp_err_t httpd_sess_trigger_close(httpd_handle_t handle, int fd) { (void)handle; (void)fd; return ESP_OK; }

static Neo_StatusType make_text(char *text, uint32 capacity, uint32 *length)
{ assert(capacity == 6144); memcpy(text, "{}", 2); *length = text_length; return text_result; }
static Neo_StatusType make_frame(boolean capture, Neo_FrameType *frame)
{
    assert(capture); ++frames; if (frame_result != NEO_OK) return frame_result;
    static const uint8 jpeg[] = {0xff, 0xd8, 0xff, 0xd9};
    *frame = (Neo_FrameType){.data = jpeg, .length = sizeof(jpeg), .token = {0, 7}}; return NEO_OK;
}
static Neo_StatusType make_photo(Neo_FrameType *frame) { ++photos; return make_frame(TRUE, frame); }
static Neo_StatusType make_stream_frame(uint64 after, uint32 timeout, Neo_FrameType *frame)
{
    assert(timeout && timeout <= 1000); last_after = after;
    if (busy_frames) { --busy_frames; return NEO_E_NOT_READY; }
    if (sequence > frame_limit) return NEO_E_IO;
    Neo_StatusType result = make_frame(TRUE, frame); if (result != NEO_OK) return result;
    frame->sequence = duplicates ? after : sequence++; if (duplicates) --duplicates;
    frame->timestamp_us = 123456; frame->profile_epoch = 7; frame->target_fps = target_fps;
    if (epoch_change_sequence != 0 && frame->sequence >= epoch_change_sequence) {
        frame->profile_epoch = changed_epoch;
        frame->target_fps = changed_fps;
    }
    if (measure_pacing) {
        assert(pacing_samples < sizeof(send_starts) / sizeof(send_starts[0]));
        /* Acquisition returns immediately, so this records the send start. */
        send_starts[pacing_samples] = now_us;
        sent_sequences[pacing_samples] = frame->sequence;
        sent_epochs[pacing_samples++] = frame->profile_epoch;
        send_step_us = frame->sequence == slow_send_sequence ? delayed_frame_us / 2U : 0;
    }
    return NEO_OK;
}
static Neo_StatusType release_frame(Neo_BufferTokenType token)
{ assert(token.slot == 0 && token.generation == 7); ++releases; return release_result; }
static Neo_StatusType select_profile(const char *name)
{ ++profile_calls; snprintf(last_profile, sizeof(last_profile), "%s", name); return selection_result; }
static void stream_opened(void) { }
static void stream_frame(const Neo_FrameType *frame, boolean sent, uint32 send_us)
{ assert(frame->length == 4); ++streamed; if (sent) ++delivered; reported_send_us = send_us; }
static const Neo_HttpPortsType ports = {
    .page = make_text, .metrics = make_text, .frame = make_frame, .release = release_frame,
    .stream_next = make_stream_frame, .photo = make_photo, .profile = make_text,
    .select_profile = select_profile, .stream_opened = stream_opened, .stream_frame = stream_frame
};
static esp_err_t request_body(const char *uri, int method, const char *body, size_t length)
{
    httpd_req_t req = {.handle = server, .fd = (int)next_fd++, .body = body, .content_len = length};
    for (unsigned i = 0; i < route_count; ++i)
        if (strcmp(routes[i].uri, uri) == 0 && routes[i].method == method) return routes[i].handler(&req);
    assert(!"Unknown route"); return ESP_FAIL;
}
static esp_err_t request(const char *uri) { return request_body(uri, HTTP_GET, NULL, 0); }
static void post(const char *body, const char *status)
{
    assert(request_body("/api/profile", HTTP_POST, body, strlen(body)) == ESP_OK);
    assert(strcmp(last_status, status) == 0);
}
static void run_worker(unsigned slot)
{
    assert(slot < 2 && tasks[slot].entry); Task task = tasks[slot]; tasks[slot] = (Task){0};
    wire_length = 0; wire[0] = '\0'; task.entry(task.arg);
}
static void start_stream(void) { sequence = 1; assert(request("/stream") == ESP_OK); }

static void pacing_run(uint16 fps, uint64 count)
{
    target_fps = fps;
    frame_limit = count;
    pacing_samples = 0;
    measure_pacing = TRUE;
    start_stream();
    run_worker(0);
    assert(pacing_samples == count);
    for (unsigned i = 1; i < pacing_samples; ++i) assert(sent_sequences[i] > sent_sequences[i - 1]);
    measure_pacing = FALSE;
    frame_limit = 3;
    target_fps = 0;
    send_step_us = 0;
}

int main(void)
{
    assert(HttpSrv_Stop() == NEO_OK && HttpSrv_Start(NULL) == NEO_E_PARAM);
    Neo_HttpPortsType invalid = ports; invalid.stream_next = NULL; assert(HttpSrv_Start(&invalid) == NEO_E_PARAM);
    fail_mutex = 1; assert(HttpSrv_Start(&ports) == NEO_E_MEMORY); fail_mutex = 0;
    fail_start = 1; assert(HttpSrv_Start(&ports) == NEO_E_IO && !mutex_count); fail_start = 0;
    fail_route = 1; assert(HttpSrv_Start(&ports) == NEO_E_IO && !mutex_count); fail_route = 0;
    assert(HttpSrv_Start(&ports) == NEO_OK && route_count == 7 && HttpSrv_Start(&ports) == NEO_E_STATE);
    assert(request("/") == ESP_OK && strcmp(last_type, "text/html") == 0);
    text_length = 6000; assert(request("/") == ESP_OK);
    text_length = 6144; assert(request("/metrics") == ESP_OK && errors == 1); text_length = 2;
    text_result = NEO_E_IO; assert(request("/api/profile") == ESP_OK && errors == 2); text_result = NEO_OK;
    assert(request("/api/profile") == ESP_OK);
    post(" { \"profile\" : \"smooth\" } \n", "202 Accepted"); assert(strcmp(last_profile, "smooth") == 0);
    const char *invalid_json[] = {"", "{}", "{\"profile\":\"\"}", "{\"profile\":12}",
        "{\"profile\":\"smooth\",\"register\":5}", "{\"profile\":\"smooth\"}x",
        "{\"profile\":\"bad\\nname\"}", "{\"profile\":\"abcdefghijklmnopqrstuvwxyz012345\"}"};
    unsigned selected = profile_calls;
    for (unsigned i = 0; i < sizeof(invalid_json)/sizeof(invalid_json[0]); ++i) post(invalid_json[i], "400 Bad Request");
    assert(profile_calls == selected);
    char oversized[130]; memset(oversized, ' ', sizeof(oversized));
    assert(request_body("/api/profile", HTTP_POST, oversized, sizeof(oversized)) == ESP_OK);
    assert(strcmp(last_status, "400 Bad Request") == 0);
    selection_result = NEO_E_BUSY; post("{\"profile\":\"smooth\"}", "409 Conflict");
    selection_result = NEO_E_PARAM; post("{\"profile\":\"unknown\"}", "400 Bad Request"); selection_result = NEO_OK;
    fail_recv = 1; post("{\"profile\":\"smooth\"}", "400 Bad Request"); fail_recv = 0;

    unsigned before = frames;
    assert(request("/capture") == ESP_OK && frames == before);
    assert(request("/photo") == ESP_OK && strcmp(last_status, "503 Service Unavailable") == 0);
    start_stream(); assert(request("/stream") == ESP_OK && strcmp(last_status, "503 Service Unavailable") == 0);
    assert(request("/metrics") == ESP_OK); run_worker(0); assert(frames == before + 1 && releases == 1);
    busy_frames = 1; duplicates = 1; target_fps = 10; uint64 started = now_us;
    run_worker(1); assert(delivered == 3 && streamed == 3 && last_after == 3);
    assert(now_us - started >= 300000U && releases == 5);
    assert(strstr(wire, "X-Sequence: 1") && strstr(wire, "X-Capture-Time-Us: 123456") && strstr(wire, "X-Profile-Epoch: 7"));
    target_fps = 0; assert(request("/photo") == ESP_OK); run_worker(0); assert(photos == 1 && releases == 6);

    blocked = 1; start_stream(); started = now_us; before = releases; unsigned previous_selects = selects;
    run_worker(0); assert(now_us - started == 500000U && reported_send_us == 500000U);
    assert(releases == before + 1 && selects > previous_selects); blocked = 0;
    partial = 1; send_step_us = 100000; start_stream(); started = now_us; run_worker(0);
    assert(now_us - started == 500000U && reported_send_us == 500000U); partial = 0; send_step_us = 0;
    blocked = 1; assert(request("/capture") == ESP_OK); started = now_us; run_worker(0);
    assert(now_us - started == 5000000U); blocked = 0;
    hard_error = 1; assert(request("/photo") == ESP_OK); before = releases; run_worker(0);
    assert(releases == before + 1); hard_error = 0;
    fail_begin = 1; assert(request("/stream") == ESP_FAIL); fail_begin = 0;
    fail_task = 1; assert(request("/stream") == ESP_OK); fail_task = 0;
    frame_result = NEO_E_BUSY; assert(request("/capture") == ESP_OK); before = releases; run_worker(0);
    assert(releases == before && strcmp(last_status, "503 Service Unavailable") == 0);
    frame_result = NEO_E_IO; assert(request("/photo") == ESP_OK); run_worker(0); assert(releases == before); frame_result = NEO_OK;
    fail_type = 1; start_stream(); run_worker(0); fail_type = 0;
    override_failures = 1; assert(request("/capture") == ESP_OK); run_worker(0);
    completion_failures = 1; assert(request("/photo") == ESP_OK); run_worker(0);
    release_result = NEO_E_IO; assert(request("/capture") == ESP_OK); run_worker(0); release_result = NEO_OK;

    /* Realistic 100 Hz scheduling preserves average rate despite 10 ms jitter. */
    fake_http_tick_ms = 10;
    pacing_run(30, 301);
    uint64 elapsed = send_starts[300] - send_starts[0];
    assert(elapsed >= 9990000U && elapsed <= 10010000U);
    for (unsigned i = 1; i < pacing_samples; ++i)
        assert(send_starts[i] - send_starts[i - 1] >= 30000U && send_starts[i] - send_starts[i - 1] <= 40000U);
    pacing_run(60, 601);
    elapsed = send_starts[600] - send_starts[0];
    assert(elapsed >= 9990000U && elapsed <= 10010000U);
    for (unsigned i = 1; i < pacing_samples; ++i)
        assert(send_starts[i] - send_starts[i - 1] >= 10000U && send_starts[i] - send_starts[i - 1] <= 20000U);
    /* A slow successful write establishes a fresh phase, with no catch-up burst. */
    slow_send_sequence = 4; delayed_frame_us = 120000;
    pacing_run(30, 10);
    assert(send_starts[4] - send_starts[3] >= 150000U);
    for (unsigned i = 5; i < pacing_samples; ++i)
        assert(send_starts[i] - send_starts[i - 1] >= 30000U);
    slow_send_sequence = 0; delayed_frame_us = 0;
    /* The new epoch discards the old fractional phase even with equal FPS. */
    epoch_change_sequence = 2; changed_fps = 30;
    pacing_run(30, 5);
    assert(sent_epochs[0] == 7 && sent_epochs[1] == changed_epoch);
    assert(send_starts[2] - send_starts[1] == 40000U);
    epoch_change_sequence = 0;
    fake_http_tick_ms = 1;

    start_stream(); assert(request("/photo") == ESP_OK); unsigned previous_stops = stops;
    assert(HttpSrv_Stop() == NEO_E_TIMEOUT && stops == previous_stops && mutex_count == 1);
    run_worker(0); assert(HttpSrv_Stop() == NEO_E_TIMEOUT && stops == previous_stops);
    assert(request("/metrics") == ESP_OK && strcmp(last_status, "503 Service Unavailable") == 0);
    run_worker(1); fail_stop = 1; assert(HttpSrv_Stop() == NEO_E_IO && mutex_count == 1); fail_stop = 0;
    assert(HttpSrv_Stop() == NEO_OK && !mutex_count && HttpSrv_Stop() == NEO_OK);
    assert(HttpSrv_Start(&ports) == NEO_OK); now_us = UINT64_MAX - 1000U;
    assert(request("/capture") == ESP_OK); run_worker(0); assert(HttpSrv_Stop() == NEO_OK);
    assert(asyncs == completes && deletes + 1 == asyncs && send_calls > 0);
    puts("HTTP profile, bounded workers and absolute send deadline tests passed"); return 0;
}
