#include "CamIo_Cfg.h"

/* Seeed XIAO ESP32-S3 Sense. SCCB carries register control, not image data. */
const CamIo_ConfigType CamIo_Config = {
    .pin_pwdn = -1,
    .pin_reset = -1,
    .pin_xclk = 10U,
    .pin_sccb_sda = 40U,
    .pin_sccb_scl = 39U,
    .pin_data = {15U, 17U, 18U, 16U, 14U, 12U, 11U, 48U},
    .pin_vsync = 38U,
    .pin_href = 47U,
    .pin_pclk = 13U,
    .xclk_hz = 20000000U,
    .width = 640U,
    .height = 480U,
    .jpeg_quality = 12U,
    .native_buffers = 2U,
};
