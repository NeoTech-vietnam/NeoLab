/* Host test: link the real pool/camera logic with these backend fakes.
 * Backend failures and ownership assertions exercise the actual release paths. */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "CamIo.h"
#include "CamIo_Backend.h"
#include "MediaBuf.h"
#include "MediaBuf_Backend.h"
#include "EbOs.h"
#include "LocalTime.h"

struct EbOs_Mutex {
    boolean locked;
};

static uint32 allocations;
static uint32 allocation_fail_at;
static uint32 native_gets;
static uint32 native_returns;
static uint32 mutex_creates;
static uint32 mutex_deletes;
static uint32 backend_deinits;
static boolean fail_camera_init;
static boolean fail_camera_deinit;
static boolean fail_camera_get;
static boolean valid_jpeg = TRUE;
static uint32 jpeg_length = 16U;
static uint8 native_bytes[16];

Neo_StatusType EbOs_MutexCreate(EbOs_MutexType *out)
{
    *out = calloc(1U, sizeof(**out));
    if (*out != NULL_PTR) {
        ++mutex_creates;
    }
    return *out == NULL_PTR ? NEO_E_MEMORY : NEO_OK;
}

Neo_StatusType EbOs_MutexLock(EbOs_MutexType mutex, uint32 timeout_ms)
{
    (void)timeout_ms;
    assert(mutex != NULL_PTR);
    if (mutex->locked) {
        return NEO_E_BUSY;
    }
    mutex->locked = TRUE;
    return NEO_OK;
}

void EbOs_MutexUnlock(EbOs_MutexType mutex)
{
    assert(mutex->locked);
    mutex->locked = FALSE;
}

void EbOs_MutexDelete(EbOs_MutexType mutex)
{
    assert(!mutex->locked);
    ++mutex_deletes;
    free(mutex);
}

uint64 LocalTime_GetUs(void)
{
    static uint64 now;
    now += 4000U;
    return now;
}

uint8 *MediaBuf_BackendAllocate(uint32 size)
{
    ++allocations;
    if (allocation_fail_at == allocations) {
        return NULL_PTR;
    }
    return malloc(size);
}

void MediaBuf_BackendFree(uint8 *data)
{
    assert(data != NULL_PTR);
    free(data);
}

Neo_StatusType CamIo_BackendInit(const CamIo_ConfigType *config,
                               Neo_CameraInfoType *info)
{
    assert(config->width == 640U && config->height == 480U);
    assert(config->jpeg_quality == 12U && config->native_buffers == 2U);
    *info = (Neo_CameraInfoType){640U, 480U, 0x3660U, "OV3660"};
    return fail_camera_init ? NEO_E_IO : NEO_OK;
}

Neo_StatusType CamIo_BackendDeinit(void)
{
    ++backend_deinits;
    return fail_camera_deinit ? NEO_E_IO : NEO_OK;
}

Neo_StatusType CamIo_BackendAcquire(CamIo_BackendFrameType *frame)
{
    ++native_gets;
    if (fail_camera_get) {
        return NEO_E_TIMEOUT;
    }
    memset(native_bytes, 0xA5, sizeof(native_bytes));
    *frame = (CamIo_BackendFrameType){
        .data = native_bytes,
        .length = jpeg_length,
        .width = 640U,
        .height = 480U,
        .timestamp_us = 1234567U,
        .jpeg = valid_jpeg,
        .native_handle = native_bytes,
    };
    return NEO_OK;
}

void CamIo_BackendReturn(CamIo_BackendFrameType *frame)
{
    assert(frame->native_handle == native_bytes);
    ++native_returns;
    frame->native_handle = NULL_PTR;
    memset(native_bytes, 0, sizeof(native_bytes));
}

int main(void)
{
    uint8 *bytes;
    Neo_BufferTokenType first, second, reused;
    Neo_FrameType frame;
    Neo_CameraInfoType info;

    assert(CamIo_ValidateConfig(&CamIo_Config) == NEO_OK);
    assert(CamIo_ValidateConfig(NULL_PTR) == NEO_E_PARAM);
    CamIo_ConfigType bad_config = CamIo_Config;
    bad_config.pin_data[0] = bad_config.pin_xclk;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.pin_pclk = 49U;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.width = 1280U;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.pin_pwdn = -2;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.pin_xclk = -1;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.pin_data[4] = -1;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.pin_xclk = 0U;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_OK);
    bad_config = CamIo_Config;
    bad_config.pin_pclk = 48U; /* In range, but aliases existing data pin. */
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    for (sint8 missing_pin = 22; missing_pin <= 25; ++missing_pin) {
        bad_config = CamIo_Config;
        bad_config.pin_pclk = missing_pin;
        assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    }
    bad_config = CamIo_Config;
    bad_config.pin_xclk = 46;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.pin_sccb_sda = 46;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.pin_sccb_scl = 46;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.pin_pwdn = 46;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.pin_reset = 46;
    assert(CamIo_ValidateConfig(&bad_config) == NEO_E_PARAM);
    bad_config = CamIo_Config;
    bad_config.pin_data[0] = 46; /* Input-only is valid for image inputs. */
    assert(CamIo_ValidateConfig(&bad_config) == NEO_OK);

    assert(MediaBuf_Acquire(&bytes, &first) == NEO_E_NOT_READY);
    assert(MediaBuf_Acquire(NULL_PTR, &first) == NEO_E_PARAM);
    allocation_fail_at = 2U;
    assert(MediaBuf_Init() == NEO_E_MEMORY); /* Partial pool allocation unwinds. */
    allocation_fail_at = 0U;
    assert(MediaBuf_Init() == NEO_OK);
    assert(MediaBuf_Init() == NEO_OK);
    assert(MediaBuf_Acquire(&bytes, &first) == NEO_OK);
    assert(MediaBuf_Acquire(&bytes, &second) == NEO_OK);
    assert(MediaBuf_Acquire(&bytes, &reused) == NEO_E_BUSY);
    assert(MediaBuf_Deinit() == NEO_E_BUSY);
    assert(MediaBuf_Release(first) == NEO_OK);
    assert(MediaBuf_Release(first) == NEO_E_STATE);
    assert(MediaBuf_Acquire(&bytes, &reused) == NEO_OK);
    assert(first.slot == reused.slot && first.generation != reused.generation);
    assert(MediaBuf_Release(first) == NEO_E_STATE);
    assert(MediaBuf_Release(second) == NEO_OK);
    assert(MediaBuf_Release(reused) == NEO_OK);
    assert(MediaBuf_Outstanding() == 0U);
    assert(MediaBuf_Deinit() == NEO_OK);
    assert(MediaBuf_Init() == NEO_OK);
    assert(MediaBuf_Acquire(&bytes, &second) == NEO_OK);
    assert(MediaBuf_Release(first) == NEO_E_STATE); /* Old session token stays stale. */
    assert(MediaBuf_Release(second) == NEO_OK);

    assert(CamIo_Capture(&frame) == NEO_E_NOT_READY);
    fail_camera_init = TRUE;
    assert(CamIo_Init() == NEO_E_IO);
    assert(CamIo_Deinit() == NEO_OK);
    fail_camera_deinit = TRUE;
    uint32 deletes_before = mutex_deletes;
    uint32 deinits_before = backend_deinits;
    assert(CamIo_Init() == NEO_E_IO);
    assert(backend_deinits == deinits_before + 1U);
    assert(mutex_deletes == deletes_before); /* Partial-init ownership retained. */
    assert(CamIo_Capture(&frame) == NEO_E_NOT_READY);
    assert(CamIo_GetInfo(&info) == NEO_E_NOT_READY);
    assert(CamIo_Init() == NEO_E_STATE);
    assert(CamIo_Deinit() == NEO_E_IO);
    assert(mutex_deletes == deletes_before);
    fail_camera_deinit = FALSE;
    assert(CamIo_Deinit() == NEO_OK);
    assert(mutex_deletes == deletes_before + 1U);
    fail_camera_init = FALSE;
    assert(CamIo_Init() == NEO_OK);
    assert(CamIo_GetInfo(&info) == NEO_OK);
    assert(strcmp(info.sensor_name, "OV3660") == 0);
    assert(CamIo_Capture(NULL_PTR) == NEO_E_PARAM);
    assert(CamIo_Capture(&frame) == NEO_OK);
    assert(native_gets == 1U && native_returns == 1U);
    assert(frame.length == 16U && frame.capture_ms == 4U);
    assert(frame.timestamp_us == 1234567U);
    assert(frame.data[0] == 0xA5U && native_bytes[0] == 0U);
    assert(CamIo_Deinit() == NEO_E_BUSY);
    assert(CamIo_Release(frame.token) == NEO_OK);
    assert(CamIo_Release(frame.token) == NEO_E_STATE);

    fail_camera_get = TRUE;
    assert(CamIo_Capture(&frame) == NEO_E_TIMEOUT);
    assert(native_returns == 1U && MediaBuf_Outstanding() == 0U);
    fail_camera_get = FALSE;
    valid_jpeg = FALSE;
    assert(CamIo_Capture(&frame) == NEO_E_IO);
    assert(native_returns == 2U && MediaBuf_Outstanding() == 0U);
    valid_jpeg = TRUE;
    jpeg_length = MEDIA_BUF_SLOT_SIZE + 1U;
    assert(CamIo_Capture(&frame) == NEO_E_TOO_LARGE);
    assert(native_returns == 3U && MediaBuf_Outstanding() == 0U);
    jpeg_length = 16U;

    assert(MediaBuf_Acquire(&bytes, &first) == NEO_OK);
    assert(MediaBuf_Acquire(&bytes, &second) == NEO_OK);
    uint32 gets_before = native_gets;
    assert(CamIo_Capture(&frame) == NEO_E_BUSY);
    assert(native_gets == gets_before); /* Exhaustion never acquires a native frame. */
    assert(MediaBuf_Release(first) == NEO_OK);
    assert(MediaBuf_Release(second) == NEO_OK);
    fail_camera_deinit = TRUE;
    assert(CamIo_Deinit() == NEO_E_IO);
    assert(CamIo_GetInfo(&info) == NEO_OK);
    fail_camera_deinit = FALSE;
    assert(CamIo_Deinit() == NEO_OK);
    assert(CamIo_Deinit() == NEO_OK);
    assert(MediaBuf_Deinit() == NEO_OK);
    assert(mutex_creates == mutex_deletes);
    assert(MediaBuf_Deinit() == NEO_OK);
    puts("camera/pool ownership tests passed");
    return 0;
}
