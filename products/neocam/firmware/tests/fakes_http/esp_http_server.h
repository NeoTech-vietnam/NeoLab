#ifndef FAKE_ESP_HTTP_SERVER_H
#define FAKE_ESP_HTTP_SERVER_H

#include <stddef.h>
#include <sys/types.h>

typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL (-1)
#define HTTP_GET 0

typedef void *httpd_handle_t;
typedef struct httpd_req {
    httpd_handle_t handle;
    int fd;
    int asynchronous;
} httpd_req_t;
typedef esp_err_t (*httpd_uri_func_t)(httpd_req_t *);
typedef struct {
    const char *uri;
    int method;
    httpd_uri_func_t handler;
} httpd_uri_t;
typedef struct {
    unsigned short server_port;
    unsigned short send_wait_timeout;
    unsigned short recv_wait_timeout;
    unsigned stack_size;
} httpd_config_t;
#define HTTPD_DEFAULT_CONFIG() ((httpd_config_t){0})

esp_err_t httpd_start(httpd_handle_t *handle, const httpd_config_t *config);
esp_err_t httpd_stop(httpd_handle_t handle);
esp_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t *route);
esp_err_t httpd_resp_send_custom_err(httpd_req_t *req, const char *status, const char *text);
esp_err_t httpd_resp_send_500(httpd_req_t *req);
esp_err_t httpd_resp_set_type(httpd_req_t *req, const char *type);
esp_err_t httpd_resp_set_hdr(httpd_req_t *req, const char *name, const char *value);
esp_err_t httpd_resp_send(httpd_req_t *req, const char *text, ssize_t length);
esp_err_t httpd_resp_send_chunk(httpd_req_t *req, const char *text, ssize_t length);
esp_err_t httpd_req_async_handler_begin(httpd_req_t *req, httpd_req_t **copy);
esp_err_t httpd_req_async_handler_complete(httpd_req_t *req);
int httpd_req_to_sockfd(httpd_req_t *req);
esp_err_t httpd_sess_trigger_close(httpd_handle_t handle, int fd);
const char *esp_err_to_name(esp_err_t error);

#endif
