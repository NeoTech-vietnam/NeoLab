#ifndef FAKE_FREERTOS_H
#define FAKE_FREERTOS_H

#include <stdint.h>

typedef uint32_t TickType_t;
typedef int BaseType_t;
typedef unsigned UBaseType_t;
#define portMAX_DELAY UINT32_MAX
#define pdTRUE 1
#define pdFALSE 0
#define pdPASS 1
extern uint32_t fake_http_tick_ms;
#define pdMS_TO_TICKS(ms) ((TickType_t)((ms) / fake_http_tick_ms))

#endif
