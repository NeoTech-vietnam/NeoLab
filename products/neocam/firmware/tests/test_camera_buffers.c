/* Real pool and camera logic with fake heap, camera, mutex and monotonic clock. */
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

struct EbOs_Mutex { boolean locked; };
static uint32 allocations, allocation_fail_at, live_allocations;
static uint32 native_gets, native_returns, mutex_creates, mutex_deletes, backend_deinits;
static boolean fail_camera_init, fail_camera_deinit, fail_cleanup_on_init, fail_camera_get;
static boolean fail_next_lock, valid_jpeg = TRUE;
static uint32 jpeg_length = 16U;
static uint64 clock_step_us = 4000U;
static uint8 native_bytes[MEDIA_BUF_PHOTO_SLOT_SIZE + 1U];
static CamIo_ConfigType native_config;

Neo_StatusType EbOs_MutexCreate(EbOs_MutexType *out)
{
    *out = calloc(1U, sizeof(**out));
    if (*out != NULL_PTR) ++mutex_creates;
    return *out == NULL_PTR ? NEO_E_MEMORY : NEO_OK;
}
Neo_StatusType EbOs_MutexLock(EbOs_MutexType mutex, uint32 timeout_ms)
{
    (void)timeout_ms;
    assert(mutex != NULL_PTR);
    if (fail_next_lock) { fail_next_lock = FALSE; return NEO_E_TIMEOUT; }
    if (mutex->locked) return NEO_E_BUSY;
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
    now += clock_step_us;
    return now;
}
uint8 *MediaBuf_BackendAllocate(uint32 size)
{
    ++allocations;
    if (allocation_fail_at == allocations) return NULL_PTR;
    uint8 *data = malloc(size);
    if (data != NULL_PTR) ++live_allocations;
    return data;
}
void MediaBuf_BackendFree(uint8 *data)
{
    assert(data != NULL_PTR && live_allocations != 0U);
    --live_allocations;
    free(data);
}
Neo_StatusType CamIo_BackendInit(const CamIo_ConfigType *config, Neo_CameraInfoType *info)
{
    assert(CamIo_ValidateConfig(config) == NEO_OK);
    native_config = *config;
    *info = (Neo_CameraInfoType){config->width, config->height, 0x3660U, "OV3660"};
    if (fail_camera_init && fail_cleanup_on_init) fail_camera_deinit = TRUE;
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
    if (fail_camera_get) return NEO_E_TIMEOUT;
    assert(jpeg_length <= sizeof(native_bytes));
    memset(native_bytes, 0xA5, jpeg_length);
    *frame = (CamIo_BackendFrameType){
        .data = native_bytes, .length = jpeg_length,
        .width = native_config.width, .height = native_config.height,
        .timestamp_us = 1234567U, .jpeg = valid_jpeg, .native_handle = native_bytes,
    };
    return NEO_OK;
}
void CamIo_BackendReturn(CamIo_BackendFrameType *frame)
{
    assert(frame->native_handle == native_bytes);
    ++native_returns;
    frame->native_handle = NULL_PTR;
    memset(native_bytes, 0, jpeg_length);
}

static void config_checks(void)
{
    static const char *names[NEO_PROFILE_COUNT] = {"safe", "smooth", "balanced", "detail", "photo"};
    assert(Neo_ProfileGet((Neo_ProfileIdType)-1) == NULL_PTR);
    assert(Neo_ProfileGet(NEO_PROFILE_COUNT) == NULL_PTR);
    assert(Neo_ProfileFind(NULL_PTR) == NULL_PTR);
    assert(Neo_ProfileFind("") == NULL_PTR);
    assert(Neo_ProfileFind("unknown") == NULL_PTR);
    assert(Neo_ProfileFind("BALANCED") == NULL_PTR);
    assert(CamIo_ValidateConfig(&CamIo_Config) == NEO_OK);
    assert(CamIo_ValidateConfig(NULL_PTR) == NEO_E_PARAM);
    for (uint32 i = 0U; i < NEO_PROFILE_COUNT; ++i) {
        const Neo_ProfileType *profile = Neo_ProfileGet((Neo_ProfileIdType)i);
        assert(profile != NULL_PTR && profile->id == (Neo_ProfileIdType)i);
        assert(strcmp(profile->name, names[i]) == 0 && Neo_ProfileFind(names[i]) == profile);
        assert(profile->jpeg_quality <= 63U && !profile->performance_validated);
        assert((profile->id == NEO_PROFILE_PHOTO) == (profile->target_fps == 0U));
        CamIo_ConfigType config = CamIo_Config;
        config.width = profile->width;
        config.height = profile->height;
        config.jpeg_quality = profile->jpeg_quality;
        assert(CamIo_ValidateConfig(&config) == NEO_OK);
    }
    CamIo_ConfigType bad = CamIo_Config;
    bad.pin_data[0] = bad.pin_xclk;
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.pin_pclk = 49;
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.width = 1280U; /* 1280x480 is not HD. */
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.pin_pwdn = -2;
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.pin_xclk = -1;
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.pin_data[4] = -1;
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.pin_xclk = 0;
    assert(CamIo_ValidateConfig(&bad) == NEO_OK);
    for (sint8 missing = 22; missing <= 25; ++missing) {
        bad = CamIo_Config; bad.pin_pclk = missing;
        assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    }
    bad = CamIo_Config; bad.pin_xclk = 46;
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.pin_sccb_sda = 46;
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.pin_sccb_scl = 46;
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.pin_pwdn = 46;
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.pin_reset = 46;
    assert(CamIo_ValidateConfig(&bad) == NEO_E_PARAM);
    bad = CamIo_Config; bad.pin_data[0] = 46;
    assert(CamIo_ValidateConfig(&bad) == NEO_OK);
}

static void pool_checks(void)
{
    uint8 *data;
    Neo_BufferTokenType preview[MEDIA_BUF_PREVIEW_SLOT_COUNT], photo, extra;
    assert(MediaBuf_Acquire(&data, &extra) == NEO_E_NOT_READY);
    assert(MediaBuf_Acquire(NULL_PTR, &extra) == NEO_E_PARAM);
    for (uint32 failure = 1U; failure <= MEDIA_BUF_SLOT_COUNT; ++failure) {
        allocation_fail_at = allocations + failure;
        assert(MediaBuf_Init() == NEO_E_MEMORY);
        assert(live_allocations == 0U);
    }
    allocation_fail_at = 0U;
    assert(MediaBuf_Init() == NEO_OK);
    assert(MediaBuf_Init() == NEO_OK);
    assert(live_allocations == MEDIA_BUF_SLOT_COUNT);
    for (uint32 i = 0U; i < MEDIA_BUF_PREVIEW_SLOT_COUNT; ++i) {
        assert(MediaBuf_Acquire(&data, &preview[i]) == NEO_OK);
        assert(preview[i].slot < MEDIA_BUF_PREVIEW_SLOT_COUNT);
        assert(MediaBuf_Capacity(preview[i]) == MEDIA_BUF_PREVIEW_SLOT_SIZE);
    }
    assert(MediaBuf_Acquire(&data, &extra) == NEO_E_BUSY);
    assert(MediaBuf_Retain(preview[0]) == NEO_OK);
    assert(MediaBuf_PreviewReferences() == MEDIA_BUF_PREVIEW_SLOT_COUNT + 1U);
    assert(MediaBuf_AcquirePhoto(&data, &photo) == NEO_OK);
    assert(photo.slot == MEDIA_BUF_PREVIEW_SLOT_COUNT);
    assert(MediaBuf_Capacity(photo) == MEDIA_BUF_PHOTO_SLOT_SIZE);
    assert(MediaBuf_AcquirePhoto(&data, &extra) == NEO_E_BUSY);
    assert(MediaBuf_Outstanding() == MEDIA_BUF_SLOT_COUNT);
    assert(MediaBuf_References() == MEDIA_BUF_SLOT_COUNT + 1U);
    assert(MediaBuf_Deinit() == NEO_E_BUSY);
    assert(MediaBuf_Release(preview[0]) == NEO_OK);
    assert(MediaBuf_Capacity(preview[0]) != 0U);
    assert(MediaBuf_Release(preview[0]) == NEO_OK);
    assert(MediaBuf_Capacity(preview[0]) == 0U);
    assert(MediaBuf_Retain(preview[0]) == NEO_E_STATE);
    assert(MediaBuf_Release(preview[0]) == NEO_E_STATE);
    assert(MediaBuf_Acquire(&data, &extra) == NEO_OK);
    assert(extra.slot == preview[0].slot && extra.generation != preview[0].generation);
    assert(MediaBuf_Retain(preview[0]) == NEO_E_STATE);
    assert(MediaBuf_Release(extra) == NEO_OK);
    for (uint32 i = 1U; i < MEDIA_BUF_PREVIEW_SLOT_COUNT; ++i)
        assert(MediaBuf_Release(preview[i]) == NEO_OK);
    assert(MediaBuf_PreviewReferences() == 0U && MediaBuf_References() == 1U);
    assert(MediaBuf_Deinit() == NEO_E_BUSY); /* Dedicated photo still owns data. */
    assert(MediaBuf_Release(photo) == NEO_OK);
    assert(MediaBuf_Deinit() == NEO_OK);
    assert(live_allocations == 0U);
    assert(MediaBuf_Init() == NEO_OK);
    assert(MediaBuf_Acquire(&data, &extra) == NEO_OK);
    assert(MediaBuf_Release(preview[0]) == NEO_E_STATE); /* Session generation stays stale. */
    assert(MediaBuf_Release(extra) == NEO_OK);
}

static void camera_checks(void)
{
    Neo_FrameType frame;
    Neo_CameraInfoType info;
    assert(CamIo_Capture(&frame) == NEO_E_NOT_READY);
    assert(CamIo_SetProfile(NEO_PROFILE_BALANCED) == NEO_E_NOT_READY);
    assert(CamIo_SetProfile(NEO_PROFILE_COUNT) == NEO_E_PARAM);
    fail_camera_init = TRUE;
    assert(CamIo_Init() == NEO_E_IO);
    assert(CamIo_Deinit() == NEO_OK);
    fail_camera_deinit = TRUE;
    uint32 deleted = mutex_deletes;
    uint32 deinits = backend_deinits;
    assert(CamIo_Init() == NEO_E_IO);
    assert(backend_deinits == deinits + 1U && mutex_deletes == deleted);
    assert(CamIo_Capture(&frame) == NEO_E_NOT_READY);
    assert(CamIo_GetInfo(&info) == NEO_E_NOT_READY);
    assert(CamIo_Init() == NEO_E_STATE);
    assert(CamIo_Deinit() == NEO_E_IO);
    fail_camera_deinit = FALSE;
    assert(CamIo_Deinit() == NEO_OK && mutex_deletes == deleted + 1U);
    fail_camera_init = FALSE;
    assert(CamIo_Init() == NEO_OK);
    assert(CamIo_GetInfo(&info) == NEO_OK && strcmp(info.sensor_name, "OV3660") == 0);
    assert(CamIo_Capture(NULL_PTR) == NEO_E_PARAM);
    assert(CamIo_Capture(&frame) == NEO_OK);
    assert(native_gets == 1U && native_returns == 1U);
    assert(frame.length == 16U && frame.capture_ms == 4U && frame.copy_us == 4000U && frame.acquire_us == 4000U);
    assert(frame.timestamp_us == 1234567U && frame.target_fps == Neo_ProfileGet(NEO_PROFILE_SAFE)->target_fps);
    assert(frame.data[0] == 0xA5U && native_bytes[0] == 0U);
    assert(MediaBuf_Retain(frame.token) == NEO_OK);
    assert(CamIo_Deinit() == NEO_E_BUSY);
    assert(CamIo_Release(frame.token) == NEO_OK);
    assert(MediaBuf_Capacity(frame.token) != 0U);
    assert(CamIo_Release(frame.token) == NEO_OK);
    assert(CamIo_Release(frame.token) == NEO_E_STATE);

    clock_step_us = 123U;
    assert(CamIo_Capture(&frame) == NEO_OK);
    assert(frame.capture_ms == 0U && frame.acquire_us == 123U && frame.copy_us == 123U);
    assert(CamIo_Release(frame.token) == NEO_OK);
    clock_step_us = (uint64)UINT32_MAX + 1000U;
    assert(CamIo_Capture(&frame) == NEO_OK);
    assert(frame.acquire_us == UINT32_MAX && frame.copy_us == UINT32_MAX);
    assert(frame.capture_ms == (uint32)(clock_step_us / 1000U));
    assert(CamIo_Release(frame.token) == NEO_OK);
    clock_step_us = 4000U;

    uint32 returned = native_returns;
    fail_camera_get = TRUE;
    assert(CamIo_Capture(&frame) == NEO_E_TIMEOUT);
    assert(native_returns == returned && MediaBuf_Outstanding() == 0U);
    fail_camera_get = FALSE;
    valid_jpeg = FALSE;
    assert(CamIo_Capture(&frame) == NEO_E_IO);
    assert(native_returns == ++returned && MediaBuf_Outstanding() == 0U);
    valid_jpeg = TRUE;
    jpeg_length = MEDIA_BUF_PREVIEW_SLOT_SIZE + 1U;
    assert(CamIo_Capture(&frame) == NEO_E_TOO_LARGE);
    assert(native_returns == ++returned && MediaBuf_Outstanding() == 0U);
    jpeg_length = 16U;

    Neo_FrameType held[MEDIA_BUF_PREVIEW_SLOT_COUNT];
    for (uint32 i = 0U; i < MEDIA_BUF_PREVIEW_SLOT_COUNT; ++i)
        assert(CamIo_Capture(&held[i]) == NEO_OK);
    uint32 gets = native_gets;
    assert(CamIo_Capture(&frame) == NEO_E_BUSY && native_gets == gets);
    for (uint32 i = 0U; i < MEDIA_BUF_PREVIEW_SLOT_COUNT; ++i)
        assert(CamIo_Release(held[i].token) == NEO_OK);

    fail_next_lock = TRUE;
    assert(CamIo_SetProfile(NEO_PROFILE_BALANCED) == NEO_E_BUSY);
    assert(CamIo_GetInfo(&info) == NEO_OK && info.width == 640U);
    assert(CamIo_SetProfile(NEO_PROFILE_PHOTO) == NEO_OK);
    jpeg_length = 768U * 1024U;
    assert(CamIo_CapturePhoto(&frame) == NEO_OK);
    assert(frame.width == 2048U && frame.height == 1536U && frame.target_fps == 0U);
    assert(MediaBuf_PreviewReferences() == 0U && MediaBuf_References() == 1U);
    assert(CamIo_SetProfile(NEO_PROFILE_BALANCED) == NEO_OK); /* Restore with photo lease live. */
    assert(CamIo_GetInfo(&info) == NEO_OK && info.width == 1280U && info.height == 720U);
    assert(frame.data[0] == 0xA5U && frame.data[frame.length - 1U] == 0xA5U);
    assert(CamIo_Deinit() == NEO_E_BUSY);
    assert(CamIo_Release(frame.token) == NEO_OK);
    jpeg_length = MEDIA_BUF_PHOTO_SLOT_SIZE + 1U;
    assert(CamIo_CapturePhoto(&frame) == NEO_E_TOO_LARGE);
    assert(MediaBuf_Outstanding() == 0U);
    jpeg_length = 16U;

    fail_camera_deinit = TRUE;
    assert(CamIo_SetProfile(NEO_PROFILE_SAFE) == NEO_E_IO);
    assert(CamIo_GetInfo(&info) == NEO_E_NOT_READY);
    fail_camera_deinit = FALSE;
    assert(CamIo_SetProfile(NEO_PROFILE_SAFE) == NEO_OK);
    fail_camera_init = TRUE;
    fail_cleanup_on_init = TRUE;
    assert(CamIo_SetProfile(NEO_PROFILE_DETAIL) == NEO_E_IO);
    assert(CamIo_Capture(&frame) == NEO_E_NOT_READY);
    assert(CamIo_Init() == NEO_E_STATE);
    fail_camera_init = FALSE;
    fail_cleanup_on_init = FALSE;
    fail_camera_deinit = FALSE;
    assert(CamIo_SetProfile(NEO_PROFILE_SAFE) == NEO_OK);
    assert(CamIo_Capture(&frame) == NEO_OK);
    assert(CamIo_Release(frame.token) == NEO_OK);
    assert(CamIo_Deinit() == NEO_OK);
    assert(CamIo_Deinit() == NEO_OK);
}

int main(void)
{
    config_checks();
    pool_checks();
    camera_checks();
    assert(MediaBuf_Deinit() == NEO_OK);
    assert(MediaBuf_Deinit() == NEO_OK);
    assert(live_allocations == 0U && mutex_creates == mutex_deletes);
    puts("camera/pool reference, profile and ownership tests passed");
    return 0;
}
