#include "Rte.h"
#include "CamIo.h"
#include "EbOs.h"
#include "LocalTime.h"

static EbOs_MutexType diagnostic_lock;
static Neo_HttpPortsType app;
static Rte_DevicePortsType device;
static Rte_DiagnosticPortsType diagnostics;

static Neo_StatusType page(char *text, uint32 size, uint32 *length) { return app.page(text, size, length); }
static Neo_StatusType metrics(char *text, uint32 size, uint32 *length) { return app.metrics(text, size, length); }
static Neo_StatusType frame(boolean capture, Neo_FrameType *value) { return app.frame(capture, value); }
static Neo_StatusType release(Neo_BufferTokenType token) { return app.release(token); }
static void opened(void) { app.stream_opened(); }
static void streamed(const Neo_FrameType *value, boolean sent) { app.stream_frame(value, sent); }
static const Neo_HttpPortsType http_ports = {page, metrics, frame, release, opened, streamed};

Neo_StatusType Rte_Init(const Neo_HttpPortsType *application,
                       const Rte_DevicePortsType *device_ports,
                       const Rte_DiagnosticPortsType *diagnostic_ports)
{
    if (application == NULL_PTR || application->page == NULL_PTR || application->metrics == NULL_PTR ||
        application->frame == NULL_PTR || application->release == NULL_PTR ||
        application->stream_opened == NULL_PTR || application->stream_frame == NULL_PTR) return NEO_E_PARAM;
    if (device_ports == NULL_PTR || device_ports->capture == NULL_PTR || device_ports->release == NULL_PTR ||
        device_ports->info == NULL_PTR || diagnostic_ports == NULL_PTR || diagnostic_ports->init == NULL_PTR ||
        diagnostic_ports->captured == NULL_PTR || diagnostic_ports->failed == NULL_PTR ||
        diagnostic_ports->opened == NULL_PTR || diagnostic_ports->streamed == NULL_PTR ||
        diagnostic_ports->json == NULL_PTR) return NEO_E_PARAM;
    if (diagnostic_lock != NULL_PTR) return NEO_E_STATE;
    Neo_StatusType result = EbOs_MutexCreate(&diagnostic_lock);
    if (result == NEO_OK) {
        app = *application;
        device = *device_ports;
        diagnostics = *diagnostic_ports;
    }
    return result;
}

Neo_StatusType Rte_Deinit(void)
{
    EbOs_MutexDelete(diagnostic_lock);
    diagnostic_lock = NULL_PTR;
    app = (Neo_HttpPortsType){0};
    device = (Rte_DevicePortsType){0};
    diagnostics = (Rte_DiagnosticPortsType){0};
    return NEO_OK;
}

const Neo_HttpPortsType *Rte_GetHttpPorts(void) { return diagnostic_lock ? &http_ports : NULL_PTR; }
Neo_StatusType Rte_CameraCapture(Neo_FrameType *value) { return diagnostic_lock ? CamIo_Capture(value) : NEO_E_NOT_READY; }
Neo_StatusType Rte_CameraRelease(Neo_BufferTokenType token) { return diagnostic_lock ? CamIo_Release(token) : NEO_E_NOT_READY; }
Neo_StatusType Rte_CameraInfo(Neo_CameraInfoType *info) { return diagnostic_lock ? CamIo_GetInfo(info) : NEO_E_NOT_READY; }
Neo_StatusType Rte_DeviceCapture(Neo_FrameType *value) { return device.capture ? device.capture(value) : NEO_E_NOT_READY; }
Neo_StatusType Rte_DeviceRelease(Neo_BufferTokenType token) { return device.release ? device.release(token) : NEO_E_NOT_READY; }
Neo_StatusType Rte_DeviceInfo(Neo_CameraInfoType *info) { return device.info ? device.info(info) : NEO_E_NOT_READY; }
void Rte_DiagInit(const Neo_CameraInfoType *info) { if (diagnostics.init) diagnostics.init(info); }
void Rte_DiagCaptured(const Neo_FrameType *value) { if (diagnostics.captured) diagnostics.captured(value); }
void Rte_DiagFailed(void) { if (diagnostics.failed) diagnostics.failed(); }
void Rte_DiagOpened(void) { if (diagnostics.opened) diagnostics.opened(); }
void Rte_DiagStreamed(const Neo_FrameType *value, boolean sent) { if (diagnostics.streamed) diagnostics.streamed(value, sent); }
Neo_StatusType Rte_DiagJson(char *text, uint32 size, uint32 *length) { return diagnostics.json ? diagnostics.json(text, size, length) : NEO_E_NOT_READY; }
uint64 Rte_TimeUs(void) { return LocalTime_GetUs(); }
void Rte_EnterDiagnostics(void) { (void)EbOs_MutexLock(diagnostic_lock, EBOS_WAIT_FOREVER); }
void Rte_ExitDiagnostics(void) { EbOs_MutexUnlock(diagnostic_lock); }
