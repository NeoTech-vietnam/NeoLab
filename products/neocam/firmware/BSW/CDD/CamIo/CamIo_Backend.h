#ifndef CAM_IO_BACKEND_H
#define CAM_IO_BACKEND_H

#include "CamIo_Cfg.h"

/* Private view: backend owns native_handle until Return. No SDK types cross
 * this seam, which also permits native-frame lifetime tests on a host. */
typedef struct {
    const uint8 *data;
    uint32 length;
    uint32 width;
    uint32 height;
    uint64 timestamp_us;
    boolean jpeg;
    void *native_handle;
} CamIo_BackendFrameType;

Neo_StatusType CamIo_BackendInit(const CamIo_ConfigType *config,
                               Neo_CameraInfoType *info);
Neo_StatusType CamIo_BackendDeinit(void);
Neo_StatusType CamIo_BackendAcquire(CamIo_BackendFrameType *frame);
void CamIo_BackendReturn(CamIo_BackendFrameType *frame);

#endif
