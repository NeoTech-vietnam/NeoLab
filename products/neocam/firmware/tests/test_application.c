#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "CamHmi.h"
#include "CamDev.h"
#include "CamDiag.h"
#include "CamIo.h"
#include "EbOs.h"
#include "Rte.h"

struct EbOs_Mutex { boolean held; };
static uint64 now_us;
static unsigned captures, releases;
static Neo_StatusType capture_result = NEO_OK;
static const uint8 jpeg[] = {0xff, 0xd8, 0xff, 0xd9};

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
Neo_StatusType CamIo_Capture(Neo_FrameType *frame)
{
    if (capture_result != NEO_OK) return capture_result;
    ++captures;
    *frame = (Neo_FrameType){.data = jpeg, .length = sizeof(jpeg), .width = 640,
        .height = 480, .capture_ms = 42, .token = {0, captures}};
    return NEO_OK;
}
Neo_StatusType CamIo_Release(Neo_BufferTokenType token) { assert(token.generation); ++releases; return NEO_OK; }
Neo_StatusType CamIo_GetInfo(Neo_CameraInfoType *info)
{
    *info = (Neo_CameraInfoType){.width = 640, .height = 480, .sensor_pid = 0x3660,
        .sensor_name = "OV3660"};
    return NEO_OK;
}

int main(void)
{
    const Rte_DevicePortsType device = {CamDev_Capture, CamDev_Release, CamDev_GetInfo};
    const Rte_DiagnosticPortsType diagnostics = {CamDiag_Init, CamDiag_Captured, CamDiag_Failed,
        CamDiag_StreamOpened, CamDiag_StreamFrame, CamDiag_GetJson};
    assert(Rte_GetHttpPorts() == NULL_PTR);
    assert(Rte_Init(NULL_PTR, &device, &diagnostics) == NEO_E_PARAM);
    assert(Rte_Init(&CamHmi_HttpPorts, NULL_PTR, &diagnostics) == NEO_E_PARAM);
    assert(Rte_Init(&CamHmi_HttpPorts, &device, NULL_PTR) == NEO_E_PARAM);
    assert(Rte_Init(&CamHmi_HttpPorts, &device, &diagnostics) == NEO_OK);
    assert(CamHmi_Init() == NEO_OK);
    assert(Rte_Init(&CamHmi_HttpPorts, &device, &diagnostics) == NEO_E_STATE);
    const Neo_HttpPortsType *ports = Rte_GetHttpPorts();
    char text[2048];
    uint32 length;
    assert(ports->page(text, sizeof(text), &length) == NEO_OK);
    assert(strstr(text, "OV3660 acquisition"));
    assert(strstr(text, "src=/stream"));
    assert(ports->page(text, 8, &length) == NEO_E_TOO_LARGE);
    assert(ports->metrics(NULL_PTR, 0, &length) == NEO_E_PARAM);
    assert(ports->metrics(text, 2, &length) == NEO_E_TOO_LARGE);
    Neo_FrameType frame;
    assert(ports->frame(TRUE, &frame) == NEO_OK);
    assert(ports->release(frame.token) == NEO_OK);
    ports->stream_opened(); /* Time zero must be a valid first-open instant. */
    now_us = 1000000;
    assert(ports->frame(FALSE, &frame) == NEO_OK);
    ports->stream_frame(&frame, TRUE);
    assert(ports->release(frame.token) == NEO_OK);
    assert(ports->metrics(text, sizeof(text), &length) == NEO_OK);
    assert(length == strlen(text));
    assert(strstr(text, "\"captures\":1"));
    assert(strstr(text, "\"stream_frames\":1"));
    assert(strstr(text, "\"latest_capture_ms\":42"));
    assert(strstr(text, "\"stream_fps\":1.00"));
    now_us = 2000000;
    ports->stream_opened(); /* Reopening does not restart the lifetime average. */
    ports->stream_frame(&frame, FALSE);
    capture_result = NEO_E_BUSY;
    assert(ports->frame(TRUE, &frame) == NEO_E_BUSY);
    capture_result = NEO_E_IO;
    assert(ports->frame(FALSE, &frame) == NEO_E_IO);
    assert(ports->metrics(text, sizeof(text), &length) == NEO_OK);
    assert(strstr(text, "\"failures\":1"));
    assert(strstr(text, "\"stream_frames\":1"));
    assert(strstr(text, "\"stream_fps\":0.50"));
    assert(captures == releases);
    assert(CamHmi_Deinit() == NEO_OK);
    assert(Rte_Deinit() == NEO_OK);
    assert(Rte_Deinit() == NEO_OK);
    assert(Rte_GetHttpPorts() == NULL_PTR);
    return 0;
}
