#include "CamPipe_Backend.h"
#include "CamPipe_Cfg.h"
#include "LocalTime.h"
#include "WifiMgr_Cfg.h"

#include <stdatomic.h>
#include <stdbool.h>

#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static TaskHandle_t producer_task;
static atomic_bool producer_running;

static void producer(void *argument)
{
    (void)argument;
    while (CamPipe_ProducerStep()) {
        /* One actual RTOS tick: pdMS_TO_TICKS(1) may be zero at 100 Hz. */
        vTaskDelay(1);
    }
    atomic_store(&producer_running, false);
    /* No core or backend state is accessed after acknowledgement. */
    vTaskDelete(NULL);
}

Neo_StatusType CamPipe_BackendStart(void)
{
    if (atomic_load(&producer_running)) return NEO_E_STATE;
    atomic_store(&producer_running, true);
#if NEOCAM_PRODUCER_CORE >= 0
    BaseType_t created = xTaskCreatePinnedToCore(producer, "NeoCamera", CAM_PIPE_WORKER_STACK_BYTES,
        NULL, CAM_PIPE_WORKER_PRIORITY, &producer_task, NEOCAM_PRODUCER_CORE);
#else
    BaseType_t created = xTaskCreate(producer, "NeoCamera", CAM_PIPE_WORKER_STACK_BYTES,
        NULL, CAM_PIPE_WORKER_PRIORITY, &producer_task);
#endif
    if (created != pdPASS) {
        atomic_store(&producer_running, false);
        producer_task = NULL;
        return NEO_E_MEMORY;
    }
    return NEO_OK;
}

Neo_StatusType CamPipe_BackendStop(uint32 timeout_ms)
{
    uint64 deadline = LocalTime_GetUs() + (uint64)timeout_ms * 1000U;
    while (atomic_load(&producer_running)) {
        if (LocalTime_GetUs() >= deadline) return NEO_E_TIMEOUT;
        vTaskDelay(1);
    }
    producer_task = NULL;
    return NEO_OK;
}

void CamPipe_BackendSample(Neo_PipelineStatsType *stats)
{
    stats->free_psram = (uint32)heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    stats->min_free_psram = (uint32)heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);
    stats->largest_psram = (uint32)heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    stats->free_internal = (uint32)heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    stats->min_free_internal = (uint32)heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
    /* ESP-IDF reports stack high-water marks in bytes. Lifecycle control
     * quiesces status callers before Stop can retire this task handle. */
    stats->producer_stack_bytes = atomic_load(&producer_running) && producer_task != NULL ?
        (uint32)uxTaskGetStackHighWaterMark(producer_task) : 0U;
    stats->cpu_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;
    stats->psram_mhz = CONFIG_SPIRAM_SPEED;
    stats->wifi_channel = WifiMgr_Config.channel;
    stats->wifi_bandwidth_mhz = WifiMgr_Config.bandwidth_mhz;
#if defined(CONFIG_CAMERA_PSRAM_DMA) && CONFIG_CAMERA_PSRAM_DMA
    stats->psram_dma = TRUE;
#else
    stats->psram_dma = FALSE;
#endif
}
