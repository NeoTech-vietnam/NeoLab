#ifndef NEO_RTE_H
#define NEO_RTE_H

#include "Neo_HttpTypes.h"
#include "Neo_CameraTypes.h"

typedef struct {
    Neo_StatusType (*capture)(Neo_FrameType *frame);
    Neo_StatusType (*release)(Neo_BufferTokenType token);
    Neo_StatusType (*info)(Neo_CameraInfoType *info);
    Neo_StatusType (*next)(uint64 after_sequence, uint32 timeout_ms, Neo_FrameType *frame);
} Rte_DevicePortsType;
typedef struct {
    void (*init)(const Neo_CameraInfoType *info);
    void (*captured)(const Neo_FrameType *frame);
    void (*failed)(void);
    void (*opened)(void);
    void (*streamed)(const Neo_FrameType *frame, boolean sent, uint32 send_us);
    Neo_StatusType (*json)(char *text, uint32 capacity, uint32 *length);
} Rte_DiagnosticPortsType;
typedef struct {
    Neo_StatusType (*select_profile)(const char *name);
    Neo_StatusType (*photo)(Neo_FrameType *frame);
    Neo_StatusType (*profile_json)(char *text, uint32 capacity, uint32 *length);
} Rte_SupervisorPortsType;

Neo_StatusType Rte_Init(const Neo_HttpPortsType *application,
                       const Rte_DevicePortsType *device,
                       const Rte_DiagnosticPortsType *diagnostics,
                       const Rte_SupervisorPortsType *supervisor);
Neo_StatusType Rte_Deinit(void);
const Neo_HttpPortsType *Rte_GetHttpPorts(void);
Neo_StatusType Rte_CameraCapture(Neo_FrameType *frame);
Neo_StatusType Rte_CameraRelease(Neo_BufferTokenType token);
Neo_StatusType Rte_CameraInfo(Neo_CameraInfoType *info);
Neo_StatusType Rte_CameraNext(uint64 after_sequence, uint32 timeout_ms, Neo_FrameType *frame);
Neo_StatusType Rte_CameraPhoto(Neo_FrameType *frame);
Neo_StatusType Rte_CameraRequestProfile(Neo_ProfileIdType profile);
Neo_StatusType Rte_CameraStats(Neo_PipelineStatsType *stats);
Neo_StatusType Rte_DeviceCapture(Neo_FrameType *frame);
Neo_StatusType Rte_DeviceRelease(Neo_BufferTokenType token);
Neo_StatusType Rte_DeviceInfo(Neo_CameraInfoType *info);
Neo_StatusType Rte_DeviceNext(uint64 after_sequence, uint32 timeout_ms, Neo_FrameType *frame);
Neo_StatusType Rte_SelectProfile(const char *name);
Neo_StatusType Rte_Photo(Neo_FrameType *frame);
Neo_StatusType Rte_ProfileJson(char *text, uint32 capacity, uint32 *length);
void Rte_DiagInit(const Neo_CameraInfoType *info);
void Rte_DiagCaptured(const Neo_FrameType *frame);
void Rte_DiagFailed(void);
void Rte_DiagOpened(void);
void Rte_DiagStreamed(const Neo_FrameType *frame, boolean sent, uint32 send_us);
Neo_StatusType Rte_DiagJson(char *text, uint32 capacity, uint32 *length);
uint64 Rte_TimeUs(void);
void Rte_EnterDiagnostics(void);
void Rte_ExitDiagnostics(void);

#endif
