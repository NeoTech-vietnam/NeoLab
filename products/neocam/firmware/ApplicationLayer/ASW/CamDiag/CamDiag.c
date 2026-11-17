#include "CamDiag.h"
#include "Rte.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIAG_SAMPLES 64U

typedef struct {
    uint32 captures, stream_frames, failures, jpeg_bytes, capture_ms;
    uint16 width, height;
    uint64 stream_started_us;
    boolean stream_started;
    uint32 photos;
    uint32 epoch;
    uint32 send_us[DIAG_SAMPLES];
    uint32 age_us[DIAG_SAMPLES];
    uint32 interval_us[DIAG_SAMPLES];
    uint64 sent_at[DIAG_SAMPLES];
    uint64 previous_sent_us;
    uint32 sample_count;
    uint32 sample_next;
    uint64 last_sequence;
    uint64 skipped;
} CamDiag_StateType;
static CamDiag_StateType state;

void CamDiag_Init(const Neo_CameraInfoType *info)
{
    Rte_EnterDiagnostics();
    state = (CamDiag_StateType){.width = info->width, .height = info->height};
    Rte_ExitDiagnostics();
}

void CamDiag_Captured(const Neo_FrameType *frame)
{
    Rte_EnterDiagnostics();
    state.captures++;
    if (frame->target_fps == 0U) state.photos++;
    state.jpeg_bytes = frame->length;
    state.capture_ms = frame->capture_ms;
    state.width = frame->width;
    state.height = frame->height;
    Rte_ExitDiagnostics();
}

void CamDiag_Failed(void)
{
    Rte_EnterDiagnostics();
    state.failures++;
    Rte_ExitDiagnostics();
}

void CamDiag_StreamOpened(void)
{
    uint64 now = Rte_TimeUs();
    Rte_EnterDiagnostics();
    if (!state.stream_started) {
        state.stream_started_us = now;
        state.stream_started = TRUE;
    }
    Rte_ExitDiagnostics();
}

void CamDiag_StreamFrame(const Neo_FrameType *frame, boolean sent, uint32 send_us)
{
    uint64 now = Rte_TimeUs();
    Rte_EnterDiagnostics();
    state.jpeg_bytes = frame->length;
    state.width = frame->width;
    state.height = frame->height;
    if (sent) {
        if (state.epoch != frame->profile_epoch) {
            state.sample_count = 0;
            state.sample_next = 0;
            state.previous_sent_us = 0;
            state.epoch = frame->profile_epoch;
            state.last_sequence = 0;
        }
        if (state.last_sequence != 0 && frame->sequence > state.last_sequence + 1U)
            state.skipped += frame->sequence - state.last_sequence - 1U;
        state.last_sequence = frame->sequence;
        state.stream_frames++;
        uint32 slot = state.sample_next;
        state.send_us[slot] = send_us;
        uint64 age = now >= frame->timestamp_us ? now - frame->timestamp_us : 0;
        state.age_us[slot] = age > UINT32_MAX ? UINT32_MAX : (uint32)age;
        uint64 interval = state.previous_sent_us ? now - state.previous_sent_us : 0;
        state.interval_us[slot] = interval > UINT32_MAX ? UINT32_MAX : (uint32)interval;
        state.sent_at[slot] = now;
        state.previous_sent_us = now;
        state.sample_next = (slot + 1U) % DIAG_SAMPLES;
        if (state.sample_count < DIAG_SAMPLES) state.sample_count++;
    }
    Rte_ExitDiagnostics();
}

static int compare_uint32(const void *left, const void *right)
{
    uint32 a = *(const uint32 *)left, b = *(const uint32 *)right;
    return (a > b) - (a < b);
}

static uint32 percentile95(const uint32 *samples, uint32 count)
{
    if (count == 0U) return 0;
    uint32 sorted[DIAG_SAMPLES];
    memcpy(sorted, samples, count * sizeof(*samples));
    qsort(sorted, count, sizeof(sorted[0]), compare_uint32);
    return sorted[(count * 95U + 99U) / 100U - 1U];
}

Neo_StatusType CamDiag_GetJson(char *text, uint32 capacity, uint32 *length)
{
    if (text == NULL_PTR || capacity == 0U || length == NULL_PTR) return NEO_E_PARAM;
    Rte_EnterDiagnostics();
    CamDiag_StateType snapshot = state;
    Rte_ExitDiagnostics();
    uint64 now = Rte_TimeUs();
    Neo_PipelineStatsType pipeline = {0};
    Neo_StatusType pipeline_result = Rte_CameraStats(&pipeline);
    const Neo_ProfileType *profile = pipeline_result == NEO_OK ? Neo_ProfileGet(pipeline.active_profile) : NULL_PTR;
    if (profile != NULL_PTR) { snapshot.width = profile->width; snapshot.height = profile->height; }
    uint32 recent = 0;
    for (uint32 i = 0; i < snapshot.sample_count; ++i)
        if (now >= snapshot.sent_at[i] && now - snapshot.sent_at[i] < 1000000U) recent++;
    double seconds = snapshot.stream_started && now > snapshot.stream_started_us
        ? (double)(now - snapshot.stream_started_us) / 1000000.0 : 0.0;
    double fps = seconds > 0.0 ? snapshot.stream_frames / seconds : 0.0;
    int size = snprintf(text, capacity,
        "{\"width\":%u,\"height\":%u,\"jpeg_bytes\":%" PRIu32 ",\"captures\":%" PRIu32
        ",\"stream_frames\":%" PRIu32 ",\"failures\":%" PRIu32
        ",\"latest_capture_ms\":%" PRIu32 ",\"uptime_s\":%" PRIu64 ",\"stream_fps\":%.2f"
        ",\"photos\":%" PRIu32 ",\"profile\":\"%s\",\"profile_epoch\":%" PRIu32
        ",\"performance_validated\":%s,\"target_fps\":%u,\"send_fps_1s\":%" PRIu32
        ",\"capture_fps_1s\":%.2f,\"capture_frames\":%" PRIu64
        ",\"superseded_frames\":%" PRIu64 ",\"capture_failures\":%" PRIu64
        ",\"oversized_frames\":%" PRIu64 ",\"acquire_p95_us\":%" PRIu32
        ",\"copy_p95_us\":%" PRIu32 ",\"send_p95_us\":%" PRIu32
        ",\"frame_age_p95_us\":%" PRIu32 ",\"send_interval_p95_us\":%" PRIu32
        ",\"min_jpeg_bytes\":%" PRIu32 ",\"max_jpeg_bytes\":%" PRIu32
        ",\"free_psram\":%" PRIu32 ",\"largest_psram\":%" PRIu32
        ",\"free_internal\":%" PRIu32 ",\"min_free_internal\":%" PRIu32
        ",\"producer_stack_bytes\":%" PRIu32 ",\"cpu_mhz\":%" PRIu32
        ",\"psram_mhz\":%" PRIu32 ",\"psram_dma\":%s,\"min_free_psram\":%" PRIu32
        ",\"pool_outstanding\":%" PRIu32 ",\"pool_references\":%" PRIu32
        ",\"last_operation_us\":%" PRIu32 ",\"last_photo_pause_us\":%" PRIu32
        ",\"wifi_channel\":%u,\"wifi_configured_bandwidth_mhz\":%u,\"stream_skipped_frames\":%" PRIu64
        ",\"camera_error\":%u,\"restore_error\":%u}",
        snapshot.width, snapshot.height, snapshot.jpeg_bytes, snapshot.captures,
        snapshot.stream_frames, snapshot.failures, snapshot.capture_ms, now / 1000000U, fps,
        snapshot.photos, profile ? profile->name : "unavailable", pipeline.epoch,
        profile && profile->performance_validated ? "true" : "false", profile ? profile->target_fps : 0,
        recent, pipeline.capture_fps_x100 / 100.0, pipeline.captured, pipeline.superseded,
        pipeline.failures, pipeline.oversized, pipeline.acquire_p95_us, pipeline.copy_p95_us,
        percentile95(snapshot.send_us, snapshot.sample_count), percentile95(snapshot.age_us, snapshot.sample_count),
        percentile95(snapshot.interval_us, snapshot.sample_count), pipeline.min_jpeg_bytes, pipeline.max_jpeg_bytes,
        pipeline.free_psram, pipeline.largest_psram, pipeline.free_internal, pipeline.min_free_internal,
        pipeline.producer_stack_bytes, pipeline.cpu_mhz, pipeline.psram_mhz,
        pipeline.psram_dma ? "true" : "false", pipeline.min_free_psram,
        pipeline.pool_outstanding, pipeline.pool_references,
        pipeline.last_operation_us, pipeline.last_photo_pause_us,
        pipeline.wifi_channel, pipeline.wifi_bandwidth_mhz, snapshot.skipped,
        (unsigned)pipeline.last_error, (unsigned)pipeline.restore_error);
    if (size < 0 || (uint32)size >= capacity) return NEO_E_TOO_LARGE;
    *length = (uint32)size;
    return NEO_OK;
}
