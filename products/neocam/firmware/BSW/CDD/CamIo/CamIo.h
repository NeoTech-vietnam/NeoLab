#ifndef CAM_IO_H
#define CAM_IO_H

#include "Neo_CameraTypes.h"

/* Lifecycle calls belong to BootCtrl after clients have quiesced. Capture
 * is serialized internally. Successful capture transfers one pool lease to
 * the caller; the native camera framebuffer has already been returned. */
Neo_StatusType CamIo_Init(void);
Neo_StatusType CamIo_Deinit(void);
/* Reconfigure native ownership only; already copied leases stay valid. The
 * pipeline serializes mode transitions and restores the previous profile if
 * this fails. A failed transition is not ready until SetProfile succeeds. */
Neo_StatusType CamIo_SetProfile(Neo_ProfileIdType profile);
Neo_StatusType CamIo_Capture(Neo_FrameType *frame);
Neo_StatusType CamIo_CapturePhoto(Neo_FrameType *frame);
Neo_StatusType CamIo_Release(Neo_BufferTokenType token);
Neo_StatusType CamIo_GetInfo(Neo_CameraInfoType *info);

#endif
