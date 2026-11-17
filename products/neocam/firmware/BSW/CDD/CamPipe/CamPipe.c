#include "CamPipe.h"
#include "CamPipe_Backend.h"
#include "CamPipe_Cfg.h"
#include "CamIo.h"
#include "MediaBuf.h"
#include "EbOs.h"
#include "LocalTime.h"

#include <string.h>

typedef enum { CMD_NONE, CMD_PROFILE, CMD_PHOTO } CommandType;

static EbOs_MutexType mutex;
static Neo_PipelineStatsType stats;
static Neo_FrameType latest;
static boolean latest_valid;
static boolean stopping;
static boolean operation_active;
static CommandType command;
static uint64 command_deadline;
static Neo_ProfileIdType desired_profile;
static Neo_ProfileIdType stable_profile;
static uint64 sequence_counter;
static uint32 epoch_counter;
static uint64 last_native_timestamp;
static boolean has_native_timestamp;
static uint32 acquire_times[CAM_PIPE_TIMING_SAMPLES];
static uint32 copy_times[CAM_PIPE_TIMING_SAMPLES];
static uint32 timing_count;
static uint32 timing_next;
static uint64 fps_window_us;
static uint64 last_publish_us;
static uint32 fps_window_frames;
static uint32 waiting_clients;
static boolean photo_waiter;
static boolean photo_done;
static Neo_StatusType photo_result;
static Neo_FrameType photo_result_frame;
static boolean photo_result_valid;
static uint64 photo_ticket;

static void lock(void) { (void)EbOs_MutexLock(mutex, EBOS_WAIT_FOREVER); }
static void unlock(void) { EbOs_MutexUnlock(mutex); }

static boolean stop_requested(void)
{
    lock();
    boolean result = stopping;
    unlock();
    return result;
}

static uint64 deadline_from(uint32 ms)
{
    return LocalTime_GetUs() + (uint64)ms * 1000U;
}

static void release_latest(void)
{
    Neo_BufferTokenType token = {0};
    lock();
    boolean release = latest_valid;
    if (release) token = latest.token;
    latest_valid = FALSE;
    unlock();
    if (release) (void)MediaBuf_Release(token);
}

static void reset_window(void)
{
    timing_count = 0U;
    timing_next = 0U;
    fps_window_us = LocalTime_GetUs();
    fps_window_frames = 0U;
    stats.capture_fps_x100 = 0U;
    stats.min_jpeg_bytes = 0U;
    stats.max_jpeg_bytes = 0U;
}

static Neo_StatusType wait_for_preview_leases(uint64 deadline)
{
    while (MediaBuf_PreviewReferences() != 0U) {
        if (stop_requested()) return NEO_E_STATE;
        if (LocalTime_GetUs() >= deadline) return NEO_E_TIMEOUT;
        EbOs_Delay(10U);
    }
    return stop_requested() ? NEO_E_STATE : NEO_OK;
}

static Neo_StatusType warmup(Neo_ProfileIdType id, uint64 operation_deadline)
{
    const Neo_ProfileType *profile = Neo_ProfileGet(id);
    uint64 deadline = deadline_from(CAM_PIPE_WARMUP_TIMEOUT_MS);
    if (operation_deadline < deadline) deadline = operation_deadline;
    uint64 timestamp = 0U;
    boolean timestamp_valid = FALSE;
    uint32 valid = 0U;
    while (valid < CAM_PIPE_WARMUP_FRAMES) {
        if (stop_requested()) return NEO_E_STATE;
        if (LocalTime_GetUs() >= deadline) return NEO_E_TIMEOUT;
        Neo_FrameType frame;
        Neo_StatusType result = id == NEO_PROFILE_PHOTO ? CamIo_CapturePhoto(&frame) : CamIo_Capture(&frame);
        if (result != NEO_OK) return result;
        boolean correct = profile != NULL_PTR && frame.width == profile->width &&
                          frame.height == profile->height;
        boolean fresh = !timestamp_valid || frame.timestamp_us != timestamp;
        timestamp = frame.timestamp_us;
        timestamp_valid = TRUE;
        Neo_StatusType released = MediaBuf_Release(frame.token);
        if (released != NEO_OK) return released;
        if (LocalTime_GetUs() >= deadline) return NEO_E_TIMEOUT;
        if (!correct) return NEO_E_IO;
        if (fresh) ++valid;
        /* A duplicate queued native frame must not spin at producer priority. */
        EbOs_Delay(1U);
    }
    lock();
    last_native_timestamp = timestamp;
    has_native_timestamp = TRUE;
    unlock();
    return NEO_OK;
}

static Neo_StatusType activate(Neo_ProfileIdType profile, uint64 deadline)
{
    if (epoch_counter == UINT32_MAX) return NEO_E_STATE;
    Neo_StatusType result = CamIo_SetProfile(profile);
    if (result != NEO_OK) return result;
    lock();
    stats.active_profile = profile;
    stats.epoch = ++epoch_counter;
    unlock();
    return warmup(profile, deadline);
}

static void finish_photo(Neo_StatusType result, Neo_FrameType *frame, uint64 ticket)
{
    lock();
    boolean deliver = photo_waiter && photo_ticket == ticket && !stopping && result == NEO_OK;
    if (deliver) {
        photo_result_frame = *frame;
        photo_result_valid = TRUE;
    }
    if (ticket == photo_ticket) {
        photo_result = stopping ? NEO_E_STATE : result;
        photo_done = TRUE;
    }
    operation_active = FALSE;
    unlock();
    if (!deliver && result == NEO_OK) (void)MediaBuf_Release(frame->token);
}

static void process_operation(CommandType op, Neo_ProfileIdType requested, uint64 deadline)
{
    uint64 operation_start = LocalTime_GetUs();
    lock();
    Neo_ProfileIdType previous = stable_profile;
    stats.state = op == CMD_PHOTO ? NEO_PIPE_PHOTO : NEO_PIPE_SWITCHING;
    boolean abandoned = op == CMD_PHOTO && !photo_waiter;
    uint64 ticket = photo_ticket;
    unlock();
    release_latest();
    Neo_StatusType result = abandoned ? NEO_E_TIMEOUT :
        wait_for_preview_leases(deadline);
    Neo_StatusType restore = NEO_OK;
    Neo_FrameType photo = {0};
    boolean photo_valid = FALSE;
    boolean changed = FALSE;
    lock();
    if (op == CMD_PHOTO && !photo_waiter && result == NEO_OK) result = NEO_E_TIMEOUT;
    unlock();
    if (result == NEO_OK) {
        changed = TRUE; /* A failed native reinitialization also needs restoration. */
        result = activate(op == CMD_PHOTO ? NEO_PROFILE_PHOTO : requested, deadline);
        if (result == NEO_OK && op == CMD_PHOTO) {
            lock();
            boolean cancelled = !photo_waiter;
            unlock();
            result = cancelled || LocalTime_GetUs() >= deadline ? NEO_E_TIMEOUT : CamIo_CapturePhoto(&photo);
            photo_valid = result == NEO_OK;
            if (photo_valid) {
                if (sequence_counter == UINT64_MAX) result = NEO_E_STATE;
                else {
                    photo.sequence = ++sequence_counter;
                    photo.profile_epoch = epoch_counter;
                }
            }
        }
    }
    if (changed && !stop_requested() && (op == CMD_PHOTO || result != NEO_OK)) {
        /* Restoration remains necessary after the caller's deadline. It has
         * a separate bounded settling window and never transfers an orphan. */
        restore = activate(previous, deadline_from(CAM_PIPE_OPERATION_TIMEOUT_MS));
    }
    lock();
    uint64 operation_duration = LocalTime_GetUs() - operation_start;
    stats.last_operation_us = operation_duration > UINT32_MAX ? UINT32_MAX : (uint32)operation_duration;
    if (op == CMD_PHOTO) stats.last_photo_pause_us = stats.last_operation_us;
    stats.last_error = result != NEO_OK ? result : restore;
    stats.restore_error = restore;
    if (stopping) stats.state = NEO_PIPE_STOPPING;
    else if (restore != NEO_OK) stats.state = NEO_PIPE_ERROR;
    else if (result == NEO_OK || !changed || (changed && stats.active_profile == previous)) {
        stats.state = NEO_PIPE_RUNNING;
        if (op == CMD_PROFILE && result == NEO_OK) stable_profile = requested;
        reset_window();
    } else stats.state = NEO_PIPE_ERROR;
    if (result != NEO_OK || restore != NEO_OK) ++stats.failures;
    if (result == NEO_E_TOO_LARGE || restore == NEO_E_TOO_LARGE) ++stats.oversized;
    if (op != CMD_PHOTO) operation_active = FALSE;
    unlock();
    if (op == CMD_PHOTO) {
        Neo_StatusType completion = result != NEO_OK ? result : restore;
        if (photo_valid && completion != NEO_OK) {
            (void)MediaBuf_Release(photo.token);
            photo_valid = FALSE;
        }
        finish_photo(completion, &photo, ticket);
    }
}

Neo_StatusType CamPipe_Start(void)
{
    if (mutex != NULL_PTR) return NEO_E_STATE;
    Neo_CameraInfoType info;
    Neo_StatusType result = CamIo_GetInfo(&info);
    if (result != NEO_OK) return result;
    result = EbOs_MutexCreate(&mutex);
    if (result != NEO_OK) return result;
    memset(&stats, 0, sizeof(stats));
    stats.state = NEO_PIPE_SWITCHING;
    stats.active_profile = NEO_PROFILE_SAFE;
    stats.requested_profile = CAM_PIPE_DEFAULT_PROFILE;
    stats.epoch = epoch_counter;
    latest_valid = FALSE;
    stopping = FALSE;
    operation_active = FALSE;
    command = CMD_PROFILE;
    command_deadline = deadline_from(CAM_PIPE_OPERATION_TIMEOUT_MS);
    desired_profile = CAM_PIPE_DEFAULT_PROFILE;
    stable_profile = NEO_PROFILE_SAFE;
    waiting_clients = 0U;
    photo_waiter = FALSE;
    photo_done = FALSE;
    photo_result_valid = FALSE;
    has_native_timestamp = FALSE;
    last_publish_us = 0U;
    reset_window();
    result = CamPipe_BackendStart();
    if (result != NEO_OK) {
        stats.state = NEO_PIPE_STOPPED;
        command = CMD_NONE;
        EbOs_MutexDelete(mutex);
        mutex = NULL_PTR;
    }
    return result;
}

Neo_StatusType CamPipe_Stop(void)
{
    if (mutex == NULL_PTR) return NEO_OK;
    lock();
    stopping = TRUE;
    stats.state = NEO_PIPE_STOPPING;
    unlock();
    Neo_StatusType result = CamPipe_BackendStop(CAM_PIPE_OPERATION_TIMEOUT_MS);
    if (result != NEO_OK) return result;
    lock();
    if (waiting_clients != 0U) {
        unlock();
        return NEO_E_BUSY;
    }
    boolean release_photo = photo_result_valid;
    Neo_BufferTokenType photo_token = photo_result_frame.token;
    photo_result_valid = FALSE;
    stats.state = NEO_PIPE_STOPPED;
    unlock();
    if (release_photo) (void)MediaBuf_Release(photo_token);
    EbOs_MutexDelete(mutex);
    mutex = NULL_PTR;
    return NEO_OK;
}

Neo_StatusType CamPipe_RequestProfile(Neo_ProfileIdType profile)
{
    if (Neo_ProfileGet(profile) == NULL_PTR || profile == NEO_PROFILE_PHOTO) return NEO_E_PARAM;
    if (mutex == NULL_PTR) return NEO_E_NOT_READY;
    lock();
    Neo_StatusType result = NEO_OK;
    if (stopping) result = NEO_E_NOT_READY;
    else if (command != CMD_NONE || operation_active || photo_waiter) result = NEO_E_BUSY;
    else {
        desired_profile = profile;
        command = CMD_PROFILE;
        command_deadline = deadline_from(CAM_PIPE_OPERATION_TIMEOUT_MS);
        stats.requested_profile = profile;
        stats.state = NEO_PIPE_SWITCHING;
    }
    unlock();
    return result;
}

Neo_StatusType CamPipe_AcquireLatest(uint64 after_seq, uint32 timeout_ms, Neo_FrameType *frame)
{
    if (frame == NULL_PTR) return NEO_E_PARAM;
    memset(frame, 0, sizeof(*frame));
    if (mutex == NULL_PTR) return NEO_E_NOT_READY;
    uint64 deadline = deadline_from(timeout_ms);
    lock();
    ++waiting_clients;
    unlock();
    for (;;) {
        lock();
        Neo_StatusType result = NEO_OK;
        if (stopping || stats.state == NEO_PIPE_STOPPED) result = NEO_E_NOT_READY;
        else if (stats.state == NEO_PIPE_ERROR) result = stats.restore_error != NEO_OK ?
            stats.restore_error : (stats.last_error != NEO_OK ? stats.last_error : NEO_E_IO);
        else if (stats.state != NEO_PIPE_RUNNING) result = NEO_E_BUSY;
        else if (latest_valid && latest.sequence > after_seq) {
            result = MediaBuf_Retain(latest.token);
            if (result == NEO_OK) *frame = latest;
        } else result = NEO_E_TIMEOUT;
        if (result != NEO_E_TIMEOUT || LocalTime_GetUs() >= deadline) {
            --waiting_clients;
            unlock();
            return result;
        }
        unlock();
        EbOs_Delay(1U);
    }
}

Neo_StatusType CamPipe_Photo(Neo_FrameType *frame)
{
    if (frame == NULL_PTR) return NEO_E_PARAM;
    memset(frame, 0, sizeof(*frame));
    if (mutex == NULL_PTR) return NEO_E_NOT_READY;
    lock();
    if (stopping || stats.state == NEO_PIPE_STOPPED || stats.state == NEO_PIPE_ERROR) {
        unlock();
        return NEO_E_NOT_READY;
    }
    if (command != CMD_NONE || operation_active || photo_waiter) {
        unlock();
        return NEO_E_BUSY;
    }
    if (photo_ticket == UINT64_MAX) {
        unlock();
        return NEO_E_STATE;
    }
    ++photo_ticket;
    ++waiting_clients;
    photo_waiter = TRUE;
    photo_done = FALSE;
    command = CMD_PHOTO;
    command_deadline = deadline_from(CAM_PIPE_OPERATION_TIMEOUT_MS);
    stats.state = NEO_PIPE_PHOTO;
    stats.requested_profile = NEO_PROFILE_PHOTO;
    unlock();
    uint64 deadline = deadline_from(CAM_PIPE_OPERATION_TIMEOUT_MS);
    for (;;) {
        lock();
        if (photo_done || LocalTime_GetUs() >= deadline) {
            Neo_StatusType result = photo_done ? photo_result : NEO_E_TIMEOUT;
            if (photo_done && result == NEO_OK && photo_result_valid) {
                *frame = photo_result_frame;
                photo_result_valid = FALSE;
            }
            photo_waiter = FALSE;
            --waiting_clients;
            unlock();
            return result;
        }
        unlock();
        EbOs_Delay(1U);
    }
}

static uint32 percentile95(uint32 *values, uint32 count)
{
    if (count == 0U) return 0U;
    for (uint32 i = 1U; i < count; ++i) {
        uint32 value = values[i];
        uint32 j = i;
        while (j != 0U && values[j - 1U] > value) {
            values[j] = values[j - 1U];
            --j;
        }
        values[j] = value;
    }
    return values[(count * 95U + 99U) / 100U - 1U];
}

Neo_StatusType CamPipe_GetStats(Neo_PipelineStatsType *out)
{
    if (out == NULL_PTR) return NEO_E_PARAM;
    uint32 acquire[CAM_PIPE_TIMING_SAMPLES];
    uint32 copy[CAM_PIPE_TIMING_SAMPLES];
    uint32 count = 0U;
    if (mutex != NULL_PTR) {
        lock();
        *out = stats;
        if (last_publish_us == 0U || LocalTime_GetUs() - last_publish_us > 1000000U) {
            out->capture_fps_x100 = 0U;
        }
        count = timing_count;
        memcpy(acquire, acquire_times, count * sizeof(*acquire));
        memcpy(copy, copy_times, count * sizeof(*copy));
        unlock();
    } else *out = stats;
    out->acquire_p95_us = percentile95(acquire, count);
    out->copy_p95_us = percentile95(copy, count);
    out->pool_outstanding = MediaBuf_Outstanding();
    out->pool_references = MediaBuf_References();
    CamPipe_BackendSample(out);
    return NEO_OK;
}

Neo_StatusType CamPipe_GetInfo(Neo_CameraInfoType *info)
{
    return CamIo_GetInfo(info);
}

Neo_StatusType CamPipe_Release(Neo_BufferTokenType token)
{
    return MediaBuf_Release(token);
}

boolean CamPipe_ProducerStep(void)
{
    lock();
    if (stopping) {
        command = CMD_NONE;
        if (photo_waiter) {
            photo_result = NEO_E_STATE;
            photo_done = TRUE;
        }
        unlock();
        release_latest();
        return FALSE;
    }
    CommandType op = command;
    Neo_ProfileIdType requested = desired_profile;
    uint64 deadline = command_deadline;
    command = CMD_NONE;
    if (op != CMD_NONE) operation_active = TRUE;
    Neo_PipeStateType state = stats.state;
    unlock();
    if (op != CMD_NONE) {
        process_operation(op, requested, deadline);
        return TRUE;
    }
    if (state != NEO_PIPE_RUNNING) return TRUE;
    Neo_FrameType frame;
    Neo_StatusType result = CamIo_Capture(&frame);
    if (result != NEO_OK) {
        lock();
        if (result != NEO_E_BUSY) {
            ++stats.failures;
            stats.last_error = result;
            if (result == NEO_E_TOO_LARGE) ++stats.oversized;
        }
        unlock();
        return TRUE;
    }
    lock();
    if (stopping || command != CMD_NONE || (has_native_timestamp && frame.timestamp_us == last_native_timestamp)) {
        ++stats.superseded;
        unlock();
        (void)MediaBuf_Release(frame.token);
        return TRUE;
    }
    has_native_timestamp = TRUE;
    last_native_timestamp = frame.timestamp_us;
    if (sequence_counter == UINT64_MAX) {
        stats.state = NEO_PIPE_ERROR;
        stats.last_error = NEO_E_STATE;
        ++stats.failures;
        unlock();
        (void)MediaBuf_Release(frame.token);
        return TRUE;
    }
    Neo_BufferTokenType old = latest.token;
    boolean release_old = latest_valid;
    if (release_old) ++stats.superseded;
    frame.sequence = ++sequence_counter;
    frame.profile_epoch = stats.epoch;
    latest = frame;
    latest_valid = TRUE;
    ++stats.captured;
    if (stats.min_jpeg_bytes == 0U || frame.length < stats.min_jpeg_bytes) stats.min_jpeg_bytes = frame.length;
    if (frame.length > stats.max_jpeg_bytes) stats.max_jpeg_bytes = frame.length;
    acquire_times[timing_next] = frame.acquire_us;
    copy_times[timing_next] = frame.copy_us;
    timing_next = (timing_next + 1U) % CAM_PIPE_TIMING_SAMPLES;
    if (timing_count < CAM_PIPE_TIMING_SAMPLES) ++timing_count;
    ++fps_window_frames;
    uint64 now = LocalTime_GetUs();
    last_publish_us = now;
    uint64 elapsed = now - fps_window_us;
    if (elapsed >= 1000000U) {
        uint64 fps = (uint64)fps_window_frames * 100000000U / elapsed;
        stats.capture_fps_x100 = fps > UINT32_MAX ? UINT32_MAX : (uint32)fps;
        fps_window_frames = 0U;
        fps_window_us = now;
    }
    unlock();
    if (release_old) (void)MediaBuf_Release(old);
    return TRUE;
}
