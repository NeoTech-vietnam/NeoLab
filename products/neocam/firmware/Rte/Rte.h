#ifndef NEO_RTE_H
#define NEO_RTE_H

#include "Neo_HttpTypes.h"

typedef struct {
    Neo_StatusType (*capture)(Neo_FrameType *frame);
    Neo_StatusType (*release)(Neo_BufferTokenType token);
    Neo_StatusType (*info)(Neo_CameraInfoType *info);
} Rte_DevicePortsType;
typedef struct {
    void (*init)(const Neo_CameraInfoType *info);
    void (*captured)(const Neo_FrameType *frame);
    void (*failed)(void);
    void (*opened)(void);
    void (*streamed)(const Neo_FrameType *frame, boolean sent);
    Neo_StatusType (*json)(char *text, uint32 capacity, uint32 *length);
} Rte_DiagnosticPortsType;

Neo_StatusType Rte_Init(const Neo_HttpPortsType *application,
                       const Rte_DevicePortsType *device,
                       const Rte_DiagnosticPortsType *diagnostics);
Neo_StatusType Rte_Deinit(void);
const Neo_HttpPortsType *Rte_GetHttpPorts(void);
Neo_StatusType Rte_CameraCapture(Neo_FrameType *frame);
Neo_StatusType Rte_CameraRelease(Neo_BufferTokenType token);
Neo_StatusType Rte_CameraInfo(Neo_CameraInfoType *info);
Neo_StatusType Rte_DeviceCapture(Neo_FrameType *frame);
Neo_StatusType Rte_DeviceRelease(Neo_BufferTokenType token);
Neo_StatusType Rte_DeviceInfo(Neo_CameraInfoType *info);
void Rte_DiagInit(const Neo_CameraInfoType *info);
void Rte_DiagCaptured(const Neo_FrameType *frame);
void Rte_DiagFailed(void);
void Rte_DiagOpened(void);
void Rte_DiagStreamed(const Neo_FrameType *frame, boolean sent);
Neo_StatusType Rte_DiagJson(char *text, uint32 capacity, uint32 *length);
uint64 Rte_TimeUs(void);
void Rte_EnterDiagnostics(void);
void Rte_ExitDiagnostics(void);

#endif
