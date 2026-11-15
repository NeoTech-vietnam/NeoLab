#include "CamDev.h"
#include "Rte.h"
Neo_StatusType CamDev_Capture(Neo_FrameType *frame) { return Rte_CameraCapture(frame); }
Neo_StatusType CamDev_Release(Neo_BufferTokenType token) { return Rte_CameraRelease(token); }
Neo_StatusType CamDev_GetInfo(Neo_CameraInfoType *info) { return Rte_CameraInfo(info); }
