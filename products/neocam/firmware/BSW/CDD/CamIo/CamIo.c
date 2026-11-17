#include "CamIo.h"
#include "CamIo_Backend.h"
#include "MediaBuf.h"
#include "EbOs.h"
#include "LocalTime.h"

#include <string.h>

static EbOs_MutexType camera_mutex;
static Neo_CameraInfoType camera_info;
static boolean initialized;
static const Neo_ProfileType *active_profile;

static boolean supported_size(uint16 width, uint16 height)
{
    return (width == 640U && height == 480U) ||
           (width == 1280U && height == 720U) ||
           (width == 1600U && height == 1200U) ||
           (width == 2048U && height == 1536U);
}

Neo_StatusType CamIo_ValidateConfig(const CamIo_ConfigType *config)
{
    if (config == NULL_PTR || !supported_size(config->width, config->height) ||
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
    active_profile = Neo_ProfileGet(NEO_PROFILE_SAFE);
    if (active_profile == NULL_PTR) {
        EbOs_MutexDelete(camera_mutex);
        camera_mutex = NULL_PTR;
        return NEO_E_PARAM;
    }
    CamIo_ConfigType config = CamIo_Config;
    config.width = active_profile->width;
    config.height = active_profile->height;
    config.jpeg_quality = active_profile->jpeg_quality;
    result = CamIo_ValidateConfig(&config);
    if (result == NEO_OK) result = CamIo_BackendInit(&config, &camera_info);
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
        active_profile = NULL_PTR;
        memset(&camera_info, 0, sizeof(camera_info));
    }
    EbOs_MutexUnlock(camera_mutex);
    if (result == NEO_OK) {
        EbOs_MutexDelete(camera_mutex);
        camera_mutex = NULL_PTR;
    }
    return result;
}

Neo_StatusType CamIo_SetProfile(Neo_ProfileIdType profile)
{
    const Neo_ProfileType *next = Neo_ProfileGet(profile);
    if (next == NULL_PTR) return NEO_E_PARAM;
    CamIo_ConfigType config = CamIo_Config;
    config.width = next->width;
    config.height = next->height;
    config.jpeg_quality = next->jpeg_quality;
    Neo_StatusType result = CamIo_ValidateConfig(&config);
    if (result != NEO_OK) return result;
    if (camera_mutex == NULL_PTR) return NEO_E_NOT_READY;
    result = EbOs_MutexLock(camera_mutex, 0U);
    if (result != NEO_OK) return NEO_E_BUSY;
    if (initialized && active_profile == next) {
        EbOs_MutexUnlock(camera_mutex);
        return NEO_OK;
    }
    /* No native framebuffer survives Capture. Copied leases do not prevent
     * reconfiguration, in particular the photo lease during preview restore. */
    initialized = FALSE;
    result = CamIo_BackendDeinit();
    if (result == NEO_OK) {
        Neo_CameraInfoType next_info = {0};
        result = CamIo_BackendInit(&config, &next_info);
        if (result == NEO_OK) {
            camera_info = next_info;
            active_profile = next;
            initialized = TRUE;
        } else {
            /* A failed cleanup remains owned by camera_mutex for retry. */
            (void)CamIo_BackendDeinit();
        }
    }
    EbOs_MutexUnlock(camera_mutex);
    return result;
}

static Neo_StatusType capture(Neo_FrameType *frame, boolean photo)
{
    if (frame == NULL_PTR) {
        return NEO_E_PARAM;
    }
    memset(frame, 0, sizeof(*frame));
    if (camera_mutex == NULL_PTR) {
        return NEO_E_NOT_READY;
    }
    uint8 *destination;
    Neo_BufferTokenType token;
    Neo_StatusType result = photo ? MediaBuf_AcquirePhoto(&destination, &token)
                                 : MediaBuf_Acquire(&destination, &token);
    if (result != NEO_OK) {
        return result;
    }
    result = EbOs_MutexLock(camera_mutex, EBOS_WAIT_FOREVER);
    if (result != NEO_OK) {
        (void)MediaBuf_Release(token);
        return result;
    }
    /* Recheck after taking the operation lock in case a mode change failed. */
    if (!initialized) {
        EbOs_MutexUnlock(camera_mutex);
        (void)MediaBuf_Release(token);
        return NEO_E_NOT_READY;
    }
    CamIo_BackendFrameType native = {0};
    uint64 before = LocalTime_GetUs();
    result = CamIo_BackendAcquire(&native);
    uint64 duration_us = LocalTime_GetUs() - before;
    uint64 duration_ms = duration_us / 1000U;
    if (result == NEO_OK) {
        if (!native.jpeg || native.data == NULL_PTR || native.length == 0U ||
            native.width == 0U || native.width > UINT16_MAX ||
            native.height == 0U || native.height > UINT16_MAX) {
            result = NEO_E_IO;
        } else if (native.length > MediaBuf_Capacity(token)) {
            result = NEO_E_TOO_LARGE;
        } else {
            uint64 copy_started = LocalTime_GetUs();
            memcpy(destination, native.data, native.length);
            uint64 copy_duration = LocalTime_GetUs() - copy_started;
            frame->data = destination;
            frame->length = native.length;
            frame->width = (uint16)native.width;
            frame->height = (uint16)native.height;
            frame->capture_ms = duration_ms > UINT32_MAX ? UINT32_MAX : (uint32)duration_ms;
            frame->acquire_us = duration_us > UINT32_MAX ? UINT32_MAX : (uint32)duration_us;
            frame->copy_us = copy_duration > UINT32_MAX ? UINT32_MAX : (uint32)copy_duration;
            frame->target_fps = active_profile->target_fps;
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

Neo_StatusType CamIo_Capture(Neo_FrameType *frame) { return capture(frame, FALSE); }
Neo_StatusType CamIo_CapturePhoto(Neo_FrameType *frame) { return capture(frame, TRUE); }

Neo_StatusType CamIo_Release(Neo_BufferTokenType token)
{
    return MediaBuf_Release(token);
}

Neo_StatusType CamIo_GetInfo(Neo_CameraInfoType *info)
{
    if (info == NULL_PTR) {
        return NEO_E_PARAM;
    }
    if (camera_mutex == NULL_PTR) {
        return NEO_E_NOT_READY;
    }
    Neo_StatusType result = EbOs_MutexLock(camera_mutex, EBOS_WAIT_FOREVER);
    if (result == NEO_OK) {
        if (initialized) *info = camera_info;
        else result = NEO_E_NOT_READY;
        EbOs_MutexUnlock(camera_mutex);
    }
    return result;
}
