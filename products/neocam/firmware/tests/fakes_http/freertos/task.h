#ifndef FAKE_FREERTOS_TASK_H
#define FAKE_FREERTOS_TASK_H

#include "FreeRTOS.h"

typedef void *TaskHandle_t;
typedef void (*TaskFunction_t)(void *);
BaseType_t xTaskCreate(TaskFunction_t entry, const char *name, uint32_t stack,
                       void *arg, UBaseType_t priority, TaskHandle_t *task);
void vTaskDelay(TickType_t ticks);
void vTaskDelete(TaskHandle_t task);

#endif
