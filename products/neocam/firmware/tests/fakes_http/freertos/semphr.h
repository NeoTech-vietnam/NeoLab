#ifndef FAKE_FREERTOS_SEMPHR_H
#define FAKE_FREERTOS_SEMPHR_H

#include "FreeRTOS.h"

typedef struct FakeHttpMutex *SemaphoreHandle_t;
SemaphoreHandle_t xSemaphoreCreateMutex(void);
BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t wait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex);
void vSemaphoreDelete(SemaphoreHandle_t mutex);

#endif
