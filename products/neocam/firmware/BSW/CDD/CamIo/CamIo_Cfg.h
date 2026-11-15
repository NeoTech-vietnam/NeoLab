#ifndef CAM_IO_CFG_H
#define CAM_IO_CFG_H

#include "Neo_Types.h"

typedef struct {
    sint8 pin_pwdn;
    sint8 pin_reset;
    sint8 pin_xclk;
    sint8 pin_sccb_sda;
    sint8 pin_sccb_scl;
    sint8 pin_data[8];
    sint8 pin_vsync;
    sint8 pin_href;
    sint8 pin_pclk;
    uint32 xclk_hz;
    uint16 width;
    uint16 height;
    uint8 jpeg_quality;
    uint8 native_buffers;
} CamIo_ConfigType;

extern const CamIo_ConfigType CamIo_Config;
Neo_StatusType CamIo_ValidateConfig(const CamIo_ConfigType *config);

#endif
