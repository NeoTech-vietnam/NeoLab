#include "CamDev.h"
#include "Rte.h"
Neo_StatusType CamDev_Capture(Neo_FrameType *frame) { return Rte_CameraCapture(frame); }
Neo_StatusType CamDev_Release(Neo_BufferTokenType token) { return Rte_CameraRelease(token); }
Neo_StatusType CamDev_GetInfo(Neo_CameraInfoType *info) { return Rte_CameraInfo(info); }
Neo_StatusType CamDev_Next(uint64 after, uint32 timeout, Neo_FrameType *frame) { return Rte_CameraNext(after, timeout, frame); }
