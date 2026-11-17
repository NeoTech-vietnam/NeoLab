#ifndef CAM_PIPE_H
#define CAM_PIPE_H

#include "Neo_CameraTypes.h"

/* Start/Stop belong to serialized lifecycle control. Stop never invalidates
 * consumer leases: callers must return them before lower services deinitialize. */
Neo_StatusType CamPipe_Start(void);
Neo_StatusType CamPipe_Stop(void);
Neo_StatusType CamPipe_AcquireLatest(uint64 after_seq, uint32 timeout_ms, Neo_FrameType *frame);
Neo_StatusType CamPipe_Photo(Neo_FrameType *frame);
Neo_StatusType CamPipe_RequestProfile(Neo_ProfileIdType profile);
Neo_StatusType CamPipe_GetStats(Neo_PipelineStatsType *stats);
Neo_StatusType CamPipe_GetInfo(Neo_CameraInfoType *info);
Neo_StatusType CamPipe_Release(Neo_BufferTokenType token);

#endif
