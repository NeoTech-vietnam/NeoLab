#ifndef CAMSUPV_H
#define CAMSUPV_H
#include "Neo_CameraTypes.h"
Neo_StatusType CamSupv_SelectProfile(const char *name);
Neo_StatusType CamSupv_Photo(Neo_FrameType *frame);
Neo_StatusType CamSupv_ProfileJson(char *text, uint32 capacity, uint32 *length);
#endif
