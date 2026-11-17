#ifndef CAM_PIPE_BACKEND_H
#define CAM_PIPE_BACKEND_H

#include "CamPipe.h"

/* Portable producer seam. Only the backend worker (or a host test) calls it. */
boolean CamPipe_ProducerStep(void);
Neo_StatusType CamPipe_BackendStart(void);
Neo_StatusType CamPipe_BackendStop(uint32 timeout_ms);
void CamPipe_BackendSample(Neo_PipelineStatsType *stats);

#endif
