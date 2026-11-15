#include "CamIo.h"
#include "CamIo_Backend.h"
#include "MediaBuf.h"
#include "EbOs.h"
#include "LocalTime.h"

#include <string.h>

static EbOs_MutexType camera_mutex;
static Neo_CameraInfoType camera_info;
static boolean initialized;

Neo_StatusType CamIo_ValidateConfig(const CamIo_ConfigType *config)
{
    if (config == NULL_PTR || config->width != 640U || config->height != 480U ||
        config->jpeg_quality > 63U || config->native_buffers == 0U ||
        config->native_buffers > 2U || config->xclk_hz == 0U ||
        config->xclk_hz > INT32_MAX || config->pin_pwdn < -1 || config->pin_reset < -1) {
        return NEO_E_PARAM;
    }
    const sint16 pins[] = {
        config->pin_pwdn, config->pin_reset, config->pin_xclk,
        config->pin_sccb_sda, config->pin_sccb_scl,
        config->pin_data[0], config->pin_data[1], config->pin_data[2], config->pin_data[3],
        config->pin_data[4], config->pin_data[5], config->pin_data[6], config->pin_data[7],
        config->pin_vsync, config->pin_href, config->pin_pclk,
    };
    for (uint32 i = 0U; i < sizeof(pins) / sizeof(pins[0]); ++i) {
        /* ESP32-S3 has no GPIO22..25; GPIO46 is input-only. The first
         * five entries drive power/reset/clock or the SCCB open-drain bus. */
        if (pins[i] > 48 || (pins[i] >= 22 && pins[i] <= 25) ||
            (i <= 4U && pins[i] == 46)) {
            return NEO_E_PARAM;
        }
        if (pins[i] < 0) {
            if (i >= 2U) {
                return NEO_E_PARAM;
            }
            continue;
        }
        for (uint32 j = 0U; j < i; ++j) {
            if (pins[i] == pins[j]) {
                return NEO_E_PARAM;
            }
        }
    }
    return NEO_OK;
}

Neo_StatusType CamIo_Init(void)
{
    if (initialized) {
        return NEO_OK;
    }
    if (camera_mutex != NULL_PTR) {
        /* A failed initializer still owns resources until cleanup succeeds. */
        return NEO_E_STATE;
    }
    if (CamIo_ValidateConfig(&CamIo_Config) != NEO_OK) {
        return NEO_E_PARAM;
    }
    Neo_StatusType result = EbOs_MutexCreate(&camera_mutex);
    if (result != NEO_OK) {
        return result;
    }
    result = CamIo_BackendInit(&CamIo_Config, &camera_info);
    if (result != NEO_OK) {
        /* Keep the original initialization error. A failed cleanup remains
         * retryable through Deinit, without admitting captures/reinitializing. */
        if (CamIo_BackendDeinit() == NEO_OK) {
            EbOs_MutexDelete(camera_mutex);
            camera_mutex = NULL_PTR;
            memset(&camera_info, 0, sizeof(camera_info));
        }
        return result;
    }
    initialized = TRUE;
    return NEO_OK;
}

Neo_StatusType CamIo_Deinit(void)
{
    if (camera_mutex == NULL_PTR) {
        return NEO_OK;
    }
    /* Do not interrupt a native get/copy or invalidate any consumer lease. */
    Neo_StatusType result = EbOs_MutexLock(camera_mutex, 0U);
    if (result != NEO_OK) {
        return NEO_E_BUSY;
    }
    if (MediaBuf_Outstanding() != 0U) {
        EbOs_MutexUnlock(camera_mutex);
        return NEO_E_BUSY;
    }
    result = CamIo_BackendDeinit();
    if (result == NEO_OK) {
        initialized = FALSE;
        memset(&camera_info, 0, sizeof(camera_info));
    }
    EbOs_MutexUnlock(camera_mutex);
    if (result == NEO_OK) {
        EbOs_MutexDelete(camera_mutex);
        camera_mutex = NULL_PTR;
    }
    return result;
}

Neo_StatusType CamIo_Capture(Neo_FrameType *frame)
{
    if (frame == NULL_PTR) {
        return NEO_E_PARAM;
    }
    memset(frame, 0, sizeof(*frame));
    if (!initialized) {
        return NEO_E_NOT_READY;
    }
    uint8 *destination;
    Neo_BufferTokenType token;
    Neo_StatusType result = MediaBuf_Acquire(&destination, &token);
    if (result != NEO_OK) {
        return result;
    }
    result = EbOs_MutexLock(camera_mutex, EBOS_WAIT_FOREVER);
    if (result != NEO_OK) {
        (void)MediaBuf_Release(token);
        return result;
    }
    CamIo_BackendFrameType native = {0};
    uint64 before = LocalTime_GetUs();
    result = CamIo_BackendAcquire(&native);
    uint64 duration_ms = (LocalTime_GetUs() - before) / 1000U;
    if (result == NEO_OK) {
        if (!native.jpeg || native.data == NULL_PTR || native.length == 0U ||
            native.width == 0U || native.width > UINT16_MAX ||
            native.height == 0U || native.height > UINT16_MAX) {
            result = NEO_E_IO;
        } else if (native.length > MEDIA_BUF_SLOT_SIZE) {
            result = NEO_E_TOO_LARGE;
        } else {
            memcpy(destination, native.data, native.length);
            frame->data = destination;
            frame->length = native.length;
            frame->width = (uint16)native.width;
            frame->height = (uint16)native.height;
            frame->capture_ms = duration_ms > UINT32_MAX ? UINT32_MAX : (uint32)duration_ms;
            frame->timestamp_us = native.timestamp_us;
            frame->token = token;
        }
        /* Every successful native acquisition is returned exactly once,
         * including invalid/oversized frame paths, before releasing the lock. */
        CamIo_BackendReturn(&native);
    }
    EbOs_MutexUnlock(camera_mutex);
    if (result != NEO_OK) {
        (void)MediaBuf_Release(token);
    }
    return result;
}

Neo_StatusType CamIo_Release(Neo_BufferTokenType token)
{
    return MediaBuf_Release(token);
}

Neo_StatusType CamIo_GetInfo(Neo_CameraInfoType *info)
{
    if (info == NULL_PTR) {
        return NEO_E_PARAM;
    }
    if (!initialized) {
        return NEO_E_NOT_READY;
    }
    Neo_StatusType result = EbOs_MutexLock(camera_mutex, EBOS_WAIT_FOREVER);
    if (result == NEO_OK) {
        *info = camera_info;
        EbOs_MutexUnlock(camera_mutex);
    }
    return result;
}
