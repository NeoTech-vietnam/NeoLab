/* Real producer state machine and real reference-counted pool, with a manually
 * stepped worker and fake camera/clock. No native task or sensor is required. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CamPipe.h"
#include "CamPipe_Backend.h"
#include "CamPipe_Cfg.h"
#include "CamIo.h"
#include "MediaBuf.h"
#include "MediaBuf_Backend.h"
#include "EbOs.h"

struct EbOs_Mutex { boolean held; };
static uint32 live_mutexes, live_buffers;
static uint64 now_us = 1000000U;
static uint64 native_timestamp;
static boolean worker_running, stepping, auto_step, stop_timeout;
static boolean duplicate_timestamp;
static boolean camera_ready = TRUE;
static Neo_ProfileIdType native_profile = NEO_PROFILE_SAFE;
static Neo_StatusType capture_result = NEO_OK;
static Neo_StatusType start_result = NEO_OK;
static Neo_ProfileIdType fail_profile = NEO_PROFILE_COUNT;
static boolean fail_restore;
static uint32 set_calls, photo_captures;
static uint32 next_acquire_us = 12001U;
static uint32 next_copy_us = 101U;
static uint32 next_length = 32U;
static uint32 delay_capture_us;

static boolean step(void)
{
    assert(worker_running && !stepping);
    stepping = TRUE;
    boolean keep_running = CamPipe_ProducerStep();
    stepping = FALSE;
    if (!keep_running) worker_running = FALSE;
    return keep_running;
}

Neo_StatusType EbOs_MutexCreate(EbOs_MutexType *out)
{
    *out = calloc(1U, sizeof(**out));
    if (*out == NULL_PTR) return NEO_E_MEMORY;
    ++live_mutexes;
    return NEO_OK;
}

Neo_StatusType EbOs_MutexLock(EbOs_MutexType value, uint32 timeout_ms)
{
    (void)timeout_ms;
    assert(value != NULL_PTR && !value->held);
    value->held = TRUE;
    return NEO_OK;
}

void EbOs_MutexUnlock(EbOs_MutexType value)
{
    assert(value != NULL_PTR && value->held);
    value->held = FALSE;
}

void EbOs_MutexDelete(EbOs_MutexType value)
{
    assert(value != NULL_PTR && !value->held && live_mutexes != 0U);
    --live_mutexes;
    free(value);
}

void EbOs_Delay(uint32 ms)
{
    now_us += (uint64)ms * 1000U;
    if (auto_step && worker_running && !stepping) (void)step();
}

uint64 LocalTime_GetUs(void) { return now_us; }

uint8 *MediaBuf_BackendAllocate(uint32 size)
{
    uint8 *value = malloc(size);
    if (value != NULL_PTR) ++live_buffers;
    return value;
}

void MediaBuf_BackendFree(uint8 *value)
{
    assert(value != NULL_PTR && live_buffers != 0U);
    --live_buffers;
    free(value);
}

Neo_StatusType CamPipe_BackendStart(void)
{
    assert(!worker_running);
    if (start_result == NEO_OK) worker_running = TRUE;
    return start_result;
}

Neo_StatusType CamPipe_BackendStop(uint32 timeout_ms)
{
    assert(timeout_ms == CAM_PIPE_OPERATION_TIMEOUT_MS);
    if (stop_timeout) return NEO_E_TIMEOUT;
    if (worker_running) assert(!step());
    return NEO_OK;
}

void CamPipe_BackendSample(Neo_PipelineStatsType *stats)
{
    stats->free_psram = 123456U;
    stats->largest_psram = 65432U;
    stats->free_internal = 50000U;
    stats->min_free_internal = 40000U;
    stats->producer_stack_bytes = 8192U;
    stats->cpu_mhz = 160U;
    stats->psram_mhz = 40U;
    stats->psram_dma = FALSE;
}

Neo_StatusType CamIo_GetInfo(Neo_CameraInfoType *info)
{
    if (!camera_ready) return NEO_E_NOT_READY;
    const Neo_ProfileType *profile = Neo_ProfileGet(native_profile);
    *info = (Neo_CameraInfoType){profile->width, profile->height, 0x3660, "OV3660"};
    return NEO_OK;
}

Neo_StatusType CamIo_SetProfile(Neo_ProfileIdType profile)
{
    ++set_calls;
    if (profile == fail_profile || (fail_restore && profile == NEO_PROFILE_SAFE)) {
        camera_ready = FALSE;
        return NEO_E_IO;
    }
    assert(Neo_ProfileGet(profile) != NULL_PTR);
    native_profile = profile;
    camera_ready = TRUE;
    return NEO_OK;
}

static Neo_StatusType capture(boolean photo, Neo_FrameType *frame)
{
    if (!camera_ready) return NEO_E_NOT_READY;
    now_us += 33000U + delay_capture_us;
    if (capture_result != NEO_OK) return capture_result;
    uint8 *data;
    Neo_BufferTokenType token;
    Neo_StatusType result = photo ? MediaBuf_AcquirePhoto(&data, &token) : MediaBuf_Acquire(&data, &token);
    if (result != NEO_OK) return result;
    assert(next_length <= MediaBuf_Capacity(token));
    memset(data, 0x55, next_length);
    if (!duplicate_timestamp) native_timestamp += 33000U;
    const Neo_ProfileType *profile = Neo_ProfileGet(native_profile);
    *frame = (Neo_FrameType){
        .data = data, .length = next_length, .width = profile->width, .height = profile->height,
        .capture_ms = next_acquire_us / 1000U, .timestamp_us = native_timestamp, .token = token,
        .target_fps = profile->target_fps, .copy_us = next_copy_us, .acquire_us = next_acquire_us,
    };
    if (photo) ++photo_captures;
    return NEO_OK;
}

Neo_StatusType CamIo_Capture(Neo_FrameType *frame) { return capture(FALSE, frame); }
Neo_StatusType CamIo_CapturePhoto(Neo_FrameType *frame) { return capture(TRUE, frame); }

static Neo_PipelineStatsType snapshot(void)
{
    Neo_PipelineStatsType value;
    assert(CamPipe_GetStats(&value) == NEO_OK);
    return value;
}

int main(void)
{
    Neo_FrameType frame, second;
    assert(CamPipe_Stop() == NEO_OK);
    assert(CamPipe_GetStats(NULL_PTR) == NEO_E_PARAM);
    assert(CamPipe_AcquireLatest(0, 0, NULL_PTR) == NEO_E_PARAM);
    assert(CamPipe_AcquireLatest(0, 0, &frame) == NEO_E_NOT_READY);
    assert(CamPipe_Photo(NULL_PTR) == NEO_E_PARAM);
    assert(CamPipe_Photo(&frame) == NEO_E_NOT_READY);
    assert(MediaBuf_Init() == NEO_OK);
    camera_ready = FALSE;
    assert(CamPipe_Start() == NEO_E_NOT_READY);
    camera_ready = TRUE;
    start_result = NEO_E_MEMORY;
    assert(CamPipe_Start() == NEO_E_MEMORY);
    assert(live_mutexes == 1U); /* The real pool retains its own mutex. */
    start_result = NEO_OK;
    assert(CamPipe_Start() == NEO_OK);
    assert(CamPipe_Start() == NEO_E_STATE);
    assert(CamPipe_AcquireLatest(0, 0, &frame) == NEO_E_BUSY);
    assert(CamPipe_RequestProfile(NEO_PROFILE_BALANCED) == NEO_E_BUSY);
    assert(step()); /* Initial three fresh frames are discarded. */
    assert(snapshot().state == NEO_PIPE_RUNNING);
    assert(snapshot().active_profile == NEO_PROFILE_SAFE);
    assert(snapshot().captured == 0U && MediaBuf_References() == 0U);
    assert(CamPipe_AcquireLatest(0, 0, &frame) == NEO_E_TIMEOUT);
    assert(step());
    assert(snapshot().captured == 1U && MediaBuf_References() == 1U);
    assert(CamPipe_AcquireLatest(0, 0, &frame) == NEO_OK);
    assert(MediaBuf_References() == 2U);
    assert(CamPipe_AcquireLatest(frame.sequence, 0, &second) == NEO_E_TIMEOUT);
    assert(step());
    assert(CamPipe_AcquireLatest(frame.sequence, 0, &second) == NEO_OK);
    assert(second.sequence > frame.sequence && second.data != frame.data);
    assert(CamPipe_Release(frame.token) == NEO_OK);
    assert(CamPipe_Release(second.token) == NEO_OK);
    assert(MediaBuf_References() == 1U); /* Publisher still owns its reference. */
    assert(CamPipe_Stop() == NEO_OK);
    assert(MediaBuf_References() == 0U);
    assert(CamPipe_Start() == NEO_OK);
    assert(step());
    assert(step());
    uint64 captured_before = snapshot().captured;
    duplicate_timestamp = TRUE;
    assert(step());
    assert(snapshot().captured == captured_before && snapshot().superseded != 0U);
    duplicate_timestamp = FALSE;

    /* Consumers can pin old immutable frames, but an exhausted preview pool
     * drops producer work rather than blocking or reusing their storage. */
    Neo_FrameType held[3];
    for (uint32 i = 0U; i < 3U; ++i) {
        assert(CamPipe_AcquireLatest(0U, 0U, &held[i]) == NEO_OK);
        assert(step());
    }
    uint64 before_pressure = snapshot().captured;
    uint64 failures_before_pressure = snapshot().failures;
    assert(step());
    assert(snapshot().captured == before_pressure && snapshot().failures == failures_before_pressure);
    for (uint32 i = 0U; i < 3U; ++i) assert(CamPipe_Release(held[i].token) == NEO_OK);
    assert(step());

    /* Ring stores true microseconds rather than rounding acquisition to ms. */
    for (uint32 i = 1; i <= 70; ++i) {
        next_acquire_us = 10000U + i;
        next_copy_us = 100U + i;
        next_length = 30U + i;
        assert(step());
    }
    Neo_PipelineStatsType measured = snapshot();
    assert(measured.acquire_p95_us == 10067U);
    assert(measured.copy_p95_us == 167U);
    assert(measured.min_jpeg_bytes == 31U && measured.max_jpeg_bytes == 100U);
    assert(measured.capture_fps_x100 > 0U);
    assert(measured.free_psram == 123456U && measured.producer_stack_bytes == 8192U);
    now_us += 2000000U;
    assert(snapshot().capture_fps_x100 == 0U);

    capture_result = NEO_E_TOO_LARGE;
    assert(step());
    assert(snapshot().oversized == 1U && snapshot().last_error == NEO_E_TOO_LARGE);
    capture_result = NEO_OK;
    assert(CamPipe_RequestProfile(NEO_PROFILE_PHOTO) == NEO_E_PARAM);
    assert(CamPipe_RequestProfile(NEO_PROFILE_COUNT) == NEO_E_PARAM);
    assert(CamPipe_AcquireLatest(0, 0, &frame) == NEO_OK);
    assert(CamPipe_RequestProfile(NEO_PROFILE_BALANCED) == NEO_OK);
    uint32 old_set_calls = set_calls;
    assert(step()); /* A held preview reference blocks mode change until timeout. */
    assert(set_calls == old_set_calls && snapshot().last_error == NEO_E_TIMEOUT);
    assert(snapshot().state == NEO_PIPE_RUNNING && snapshot().active_profile == NEO_PROFILE_SAFE);
    assert(CamPipe_Release(frame.token) == NEO_OK);
    assert(CamPipe_RequestProfile(NEO_PROFILE_BALANCED) == NEO_OK);
    assert(step());
    assert(snapshot().active_profile == NEO_PROFILE_BALANCED && snapshot().state == NEO_PIPE_RUNNING);
    assert(step());
    assert(CamPipe_AcquireLatest(0, 0, &frame) == NEO_OK);
    assert(frame.width == 1280U && frame.height == 720U);
    assert(CamPipe_Release(frame.token) == NEO_OK);

    /* A synchronous photo request is serviced by the sole producer. Restore
     * completes before returning the dedicated photo lease to the caller. */
    auto_step = TRUE;
    assert(CamPipe_Photo(&frame) == NEO_OK);
    assert(frame.width == 2048U && frame.height == 1536U && frame.token.slot == 4U);
    assert(photo_captures >= 4U);
    assert(snapshot().last_photo_pause_us > 0U && snapshot().last_photo_pause_us < 3000000U);
    assert(snapshot().active_profile == NEO_PROFILE_BALANCED && snapshot().state == NEO_PIPE_RUNNING);
    assert(MediaBuf_PreviewReferences() == 0U && MediaBuf_References() == 1U);
    assert(CamPipe_RequestProfile(NEO_PROFILE_SAFE) == NEO_OK);
    assert(step()); /* Held photo output does not block a preview mode switch. */
    assert(snapshot().active_profile == NEO_PROFILE_SAFE);
    assert(CamPipe_Release(frame.token) == NEO_OK);
    auto_step = FALSE;

    /* Caller times out before the worker sees the command. No stale result is
     * delivered to a later caller and no orphaned photo buffer is allocated. */
    old_set_calls = set_calls;
    assert(CamPipe_Photo(&frame) == NEO_E_TIMEOUT);
    assert(CamPipe_Photo(&frame) == NEO_E_BUSY);
    assert(step());
    assert(set_calls == old_set_calls && MediaBuf_References() == 0U);
    auto_step = TRUE;
    assert(CamPipe_Photo(&frame) == NEO_OK);
    assert(CamPipe_Release(frame.token) == NEO_OK);
    auto_step = FALSE;

    fail_restore = TRUE;
    auto_step = TRUE;
    assert(CamPipe_Photo(&frame) == NEO_E_IO);
    assert(snapshot().state == NEO_PIPE_ERROR && snapshot().restore_error == NEO_E_IO);
    assert(MediaBuf_References() == 0U); /* Valid image was not orphaned by failed restoration. */
    fail_restore = FALSE;
    auto_step = FALSE;
    assert(CamPipe_RequestProfile(NEO_PROFILE_SAFE) == NEO_OK);
    assert(step());

    fail_profile = NEO_PROFILE_DETAIL;
    assert(CamPipe_RequestProfile(NEO_PROFILE_DETAIL) == NEO_OK);
    assert(step());
    assert(snapshot().last_error == NEO_E_IO && snapshot().restore_error == NEO_OK);
    assert(snapshot().active_profile == NEO_PROFILE_SAFE && snapshot().state == NEO_PIPE_RUNNING);
    fail_restore = TRUE;
    assert(CamPipe_RequestProfile(NEO_PROFILE_DETAIL) == NEO_OK);
    assert(step());
    assert(snapshot().state == NEO_PIPE_ERROR && snapshot().restore_error == NEO_E_IO);
    assert(CamPipe_AcquireLatest(0, 0, &frame) == NEO_E_IO);
    assert(CamPipe_Photo(&frame) == NEO_E_NOT_READY);
    fail_restore = FALSE;
    fail_profile = NEO_PROFILE_COUNT;
    assert(CamPipe_RequestProfile(NEO_PROFILE_SAFE) == NEO_OK);
    assert(step());
    assert(snapshot().state == NEO_PIPE_RUNNING);

    duplicate_timestamp = TRUE;
    assert(CamPipe_RequestProfile(NEO_PROFILE_SMOOTH) == NEO_OK);
    assert(step());
    assert(snapshot().state == NEO_PIPE_ERROR && snapshot().last_error == NEO_E_TIMEOUT);
    duplicate_timestamp = FALSE;
    assert(CamPipe_RequestProfile(NEO_PROFILE_SAFE) == NEO_OK);
    assert(step());
    assert(step());
    stop_timeout = TRUE;
    assert(CamPipe_Stop() == NEO_E_TIMEOUT);
    assert(CamPipe_AcquireLatest(0, 0, &frame) == NEO_E_NOT_READY);
    assert(live_mutexes == 2U && worker_running);
    stop_timeout = FALSE;
    assert(CamPipe_Stop() == NEO_OK);
    assert(CamPipe_Stop() == NEO_OK && MediaBuf_References() == 0U);
    assert(MediaBuf_Deinit() == NEO_OK);
    assert(live_mutexes == 0U && live_buffers == 0U);
    puts("Producer pipeline tests passed");
    return 0;
}
