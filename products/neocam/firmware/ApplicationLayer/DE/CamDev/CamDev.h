#ifndef CAMDEV_H
#define CAMDEV_H
#include "Neo_Types.h"
Neo_StatusType CamDev_Capture(Neo_FrameType *frame);
Neo_StatusType CamDev_Release(Neo_BufferTokenType token);
Neo_StatusType CamDev_GetInfo(Neo_CameraInfoType *info);
#endif
