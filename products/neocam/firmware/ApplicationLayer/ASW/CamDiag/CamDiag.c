#include "CamDiag.h"
#include "Rte.h"
#include <inttypes.h>
#include <stdio.h>

typedef struct {
    uint32 captures, stream_frames, failures, jpeg_bytes, capture_ms;
    uint16 width, height;
    uint64 stream_started_us;
    boolean stream_started;
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

void CamDiag_StreamFrame(const Neo_FrameType *frame, boolean sent)
{
    Rte_EnterDiagnostics();
    state.jpeg_bytes = frame->length;
    state.width = frame->width;
    state.height = frame->height;
    if (sent) state.stream_frames++;
    Rte_ExitDiagnostics();
}

Neo_StatusType CamDiag_GetJson(char *text, uint32 capacity, uint32 *length)
{
    if (text == NULL_PTR || capacity == 0U || length == NULL_PTR) return NEO_E_PARAM;
    Rte_EnterDiagnostics();
    CamDiag_StateType snapshot = state;
    Rte_ExitDiagnostics();
    uint64 now = Rte_TimeUs();
    double seconds = snapshot.stream_started && now > snapshot.stream_started_us
        ? (double)(now - snapshot.stream_started_us) / 1000000.0 : 0.0;
    double fps = seconds > 0.0 ? snapshot.stream_frames / seconds : 0.0;
    int size = snprintf(text, capacity,
        "{\"width\":%u,\"height\":%u,\"jpeg_bytes\":%" PRIu32 ",\"captures\":%" PRIu32
        ",\"stream_frames\":%" PRIu32 ",\"failures\":%" PRIu32
        ",\"latest_capture_ms\":%" PRIu32 ",\"uptime_s\":%" PRIu64 ",\"stream_fps\":%.2f}",
        snapshot.width, snapshot.height, snapshot.jpeg_bytes, snapshot.captures,
        snapshot.stream_frames, snapshot.failures, snapshot.capture_ms, now / 1000000U, fps);
    if (size < 0 || (uint32)size >= capacity) return NEO_E_TOO_LARGE;
    *length = (uint32)size;
    return NEO_OK;
}
