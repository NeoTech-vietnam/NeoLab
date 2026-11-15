#include "EbOs.h"

#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

struct EbOs_Mutex {
    SemaphoreHandle_t handle;
};

static TickType_t ticks_for(uint32 ms)
{
    uint64 ticks = ((uint64)ms * configTICK_RATE_HZ + 999U) / 1000U;
    return ticks >= portMAX_DELAY ? portMAX_DELAY - 1U : (TickType_t)ticks;
}

Neo_StatusType EbOs_MutexCreate(EbOs_MutexType *out)
{
    if (out == NULL_PTR) return NEO_E_PARAM;
    *out = NULL_PTR;
    EbOs_MutexType mutex = heap_caps_calloc(1, sizeof(*mutex), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (mutex == NULL_PTR) return NEO_E_MEMORY;
    mutex->handle = xSemaphoreCreateMutex();
    if (mutex->handle == NULL_PTR) {
        heap_caps_free(mutex);
        return NEO_E_MEMORY;
    }
    *out = mutex;
    return NEO_OK;
}

Neo_StatusType EbOs_MutexLock(EbOs_MutexType mutex, uint32 timeout_ms)
{
    if (mutex == NULL_PTR) return NEO_E_PARAM;
    TickType_t wait = timeout_ms == EBOS_WAIT_FOREVER ? portMAX_DELAY : ticks_for(timeout_ms);
    return xSemaphoreTake(mutex->handle, wait) == pdTRUE ? NEO_OK : NEO_E_TIMEOUT;
}

void EbOs_MutexUnlock(EbOs_MutexType mutex)
{
    if (mutex != NULL_PTR) (void)xSemaphoreGive(mutex->handle);
}

void EbOs_MutexDelete(EbOs_MutexType mutex)
{
    if (mutex == NULL_PTR) return;
    vSemaphoreDelete(mutex->handle);
    heap_caps_free(mutex);
}

void EbOs_Delay(uint32 ms)
{
    vTaskDelay(ticks_for(ms));
}
