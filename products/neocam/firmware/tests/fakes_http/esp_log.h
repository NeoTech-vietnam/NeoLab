#ifndef FAKE_ESP_LOG_H
#define FAKE_ESP_LOG_H

/* Consume arguments without output; keeps real diagnostic branches compiled. */
void fake_http_log(const char *tag, const char *format, ...);
#define ESP_LOGE(...) fake_http_log(__VA_ARGS__)
#define ESP_LOGW(...) fake_http_log(__VA_ARGS__)

#endif
