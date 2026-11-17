#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "CamHmi.h"
#include "CamDev.h"
#include "CamDiag.h"
#include "CamSupv.h"
#include "CamIo.h"
#include "CamPipe.h"
#include "EbOs.h"
#include "Rte.h"

struct EbOs_Mutex { boolean held; };
static uint64 now_us;
static unsigned captures, releases;
static Neo_StatusType capture_result = NEO_OK;
static const uint8 jpeg[] = {0xff, 0xd8, 0xff, 0xd9};
static Neo_ProfileIdType active_profile = NEO_PROFILE_SAFE;

Neo_StatusType EbOs_MutexCreate(EbOs_MutexType *value)
{
    *value = calloc(1, sizeof(**value));
    return *value ? NEO_OK : NEO_E_MEMORY;
}
Neo_StatusType EbOs_MutexLock(EbOs_MutexType value, uint32 wait)
{
    (void)wait;
    assert(value && !value->held);
    value->held = TRUE;
    return NEO_OK;
}
void EbOs_MutexUnlock(EbOs_MutexType value) { assert(value && value->held); value->held = FALSE; }
void EbOs_MutexDelete(EbOs_MutexType value) { free(value); }
uint64 LocalTime_GetUs(void) { return now_us; }
Neo_StatusType CamPipe_AcquireLatest(uint64 after_sequence, uint32 timeout_ms, Neo_FrameType *frame)
{
    (void)after_sequence;
    (void)timeout_ms;
    if (capture_result != NEO_OK) return capture_result;
    ++captures;
    const Neo_ProfileType *profile = Neo_ProfileGet(active_profile);
    *frame = (Neo_FrameType){.data = jpeg, .length = sizeof(jpeg), .width = 640,
        .height = 480, .capture_ms = 42, .token = {0, captures},
        .sequence = captures, .profile_epoch = 1, .target_fps = profile->target_fps};
    return NEO_OK;
}
Neo_StatusType CamPipe_Release(Neo_BufferTokenType token) { assert(token.generation); ++releases; return NEO_OK; }
Neo_StatusType CamPipe_RequestProfile(Neo_ProfileIdType profile)
{
    active_profile = profile;
    return NEO_OK;
}
Neo_StatusType CamPipe_Photo(Neo_FrameType *frame)
{
    Neo_StatusType result = CamPipe_AcquireLatest(0, 0, frame);
    if (result == NEO_OK) { frame->width = 2048; frame->height = 1536; frame->target_fps = 0; }
    return result;
}
Neo_StatusType CamPipe_GetStats(Neo_PipelineStatsType *stats)
{
    *stats = (Neo_PipelineStatsType){.state = NEO_PIPE_RUNNING, .active_profile = active_profile,
        .requested_profile = active_profile, .epoch = 1, .captured = captures, .capture_fps_x100 = 3000,
        .acquire_p95_us = 42000, .copy_p95_us = 100, .free_internal = 100000,
        .free_psram = 1000000, .min_free_internal = 88000, .min_free_psram = 800000,
        .cpu_mhz = 160, .psram_mhz = 40};
    return NEO_OK;
}
Neo_StatusType CamIo_GetInfo(Neo_CameraInfoType *info)
{
    *info = (Neo_CameraInfoType){.width = 640, .height = 480, .sensor_pid = 0x3660,
        .sensor_name = "OV3660"};
    return NEO_OK;
}

int main(void)
{
    const Rte_DevicePortsType device = {CamDev_Capture, CamDev_Release, CamDev_GetInfo, CamDev_Next};
    const Rte_DiagnosticPortsType diagnostics = {CamDiag_Init, CamDiag_Captured, CamDiag_Failed,
        CamDiag_StreamOpened, CamDiag_StreamFrame, CamDiag_GetJson};
    const Rte_SupervisorPortsType supervisor = {CamSupv_SelectProfile, CamSupv_Photo, CamSupv_ProfileJson};
    assert(Rte_GetHttpPorts() == NULL_PTR);
    assert(Rte_Init(NULL_PTR, &device, &diagnostics, &supervisor) == NEO_E_PARAM);
    assert(Rte_Init(&CamHmi_HttpPorts, NULL_PTR, &diagnostics, &supervisor) == NEO_E_PARAM);
    assert(Rte_Init(&CamHmi_HttpPorts, &device, NULL_PTR, &supervisor) == NEO_E_PARAM);
    assert(Rte_Init(&CamHmi_HttpPorts, &device, &diagnostics, NULL_PTR) == NEO_E_PARAM);
    assert(Rte_Init(&CamHmi_HttpPorts, &device, &diagnostics, &supervisor) == NEO_OK);
    assert(CamHmi_Init() == NEO_OK);
    assert(Rte_Init(&CamHmi_HttpPorts, &device, &diagnostics, &supervisor) == NEO_E_STATE);
    const Neo_HttpPortsType *ports = Rte_GetHttpPorts();
    char text[6144];
    uint32 length;
    assert(ports->page(text, sizeof(text), &length) == NEO_OK);
    assert(strstr(text, "OV3660"));
    assert(strstr(text, "src=/stream"));
    assert(strstr(text, "Download 3 MP photo"));
    assert(strstr(text, "/api/profile"));
    assert(ports->page(text, 8, &length) == NEO_E_TOO_LARGE);
    assert(ports->metrics(NULL_PTR, 0, &length) == NEO_E_PARAM);
    assert(ports->metrics(text, 2, &length) == NEO_E_TOO_LARGE);
    Neo_FrameType frame;
    assert(ports->frame(TRUE, &frame) == NEO_OK);
    assert(ports->release(frame.token) == NEO_OK);
    ports->stream_opened(); /* Time zero must be a valid first-open instant. */
    now_us = 1000000;
    assert(ports->frame(FALSE, &frame) == NEO_OK);
    ports->stream_frame(&frame, TRUE, 8000);
    assert(ports->release(frame.token) == NEO_OK);
    assert(ports->metrics(text, sizeof(text), &length) == NEO_OK);
    assert(length == strlen(text));
    assert(strstr(text, "\"captures\":1"));
    assert(strstr(text, "\"stream_frames\":1"));
    assert(strstr(text, "\"latest_capture_ms\":42"));
    assert(strstr(text, "\"stream_fps\":1.00"));
    now_us = 2000000;
    ports->stream_opened(); /* Reopening does not restart the lifetime average. */
    ports->stream_frame(&frame, FALSE, 8000);
    capture_result = NEO_E_BUSY;
    assert(ports->frame(TRUE, &frame) == NEO_E_BUSY);
    capture_result = NEO_E_IO;
    assert(ports->frame(FALSE, &frame) == NEO_E_IO);
    assert(ports->metrics(text, sizeof(text), &length) == NEO_OK);
    assert(strstr(text, "\"failures\":1"));
    assert(strstr(text, "\"stream_frames\":1"));
    assert(strstr(text, "\"stream_fps\":0.50"));
    assert(captures == releases);
    assert(ports->profile(text, sizeof(text), &length) == NEO_OK);
    assert(strstr(text, "\"performance_validated\":false"));
    assert(strstr(text, "\"id\":\"photo\""));
    assert(ports->select_profile("unknown") == NEO_E_PARAM);
    assert(ports->select_profile("photo") == NEO_E_PARAM);
    assert(ports->select_profile("balanced") == NEO_OK);
    assert(active_profile == NEO_PROFILE_BALANCED);
    capture_result = NEO_OK;
    assert(ports->stream_next(0, 100, &frame) == NEO_OK);
    assert(ports->release(frame.token) == NEO_OK);
    assert(ports->photo(&frame) == NEO_OK && frame.width == 2048 && frame.height == 1536);
    assert(ports->release(frame.token) == NEO_OK);
    assert(ports->metrics(text, sizeof(text), &length) == NEO_OK);
    assert(strstr(text, "\"photos\":1"));
    assert(strstr(text, "\"cpu_mhz\":160"));
    assert(strstr(text, "\"send_p95_us\":8000"));
    assert(captures == releases);
    assert(CamHmi_Deinit() == NEO_OK);
    assert(Rte_Deinit() == NEO_OK);
    assert(Rte_Deinit() == NEO_OK);
    assert(Rte_GetHttpPorts() == NULL_PTR);
    return 0;
}
