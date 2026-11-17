#include "CamIo_Backend.h"

#include <stdio.h>

#include "esp_camera.h"
#include "esp_log.h"
#include "esp_psram.h"

static boolean backend_initialized;

Neo_StatusType CamIo_BackendInit(const CamIo_ConfigType *config,
                               Neo_CameraInfoType *info)
{
    if (backend_initialized) {
        return NEO_E_STATE;
    }
    if (!esp_psram_is_initialized()) {
        ESP_LOGE("CamIo", "PSRAM unavailable");
        return NEO_E_MEMORY;
    }
    framesize_t framesize;
    if (config->width == 640U && config->height == 480U) framesize = FRAMESIZE_VGA;
    else if (config->width == 1280U && config->height == 720U) framesize = FRAMESIZE_HD;
    else if (config->width == 1600U && config->height == 1200U) framesize = FRAMESIZE_UXGA;
    else if (config->width == 2048U && config->height == 1536U) framesize = FRAMESIZE_QXGA;
    else return NEO_E_PARAM;
    const camera_config_t native = {
        .pin_pwdn = config->pin_pwdn,
        .pin_reset = config->pin_reset,
        .pin_xclk = config->pin_xclk,
        .pin_sccb_sda = config->pin_sccb_sda,
        .pin_sccb_scl = config->pin_sccb_scl,
        .pin_d0 = config->pin_data[0],
        .pin_d1 = config->pin_data[1],
        .pin_d2 = config->pin_data[2],
        .pin_d3 = config->pin_data[3],
        .pin_d4 = config->pin_data[4],
        .pin_d5 = config->pin_data[5],
        .pin_d6 = config->pin_data[6],
        .pin_d7 = config->pin_data[7],
        .pin_vsync = config->pin_vsync,
        .pin_href = config->pin_href,
        .pin_pclk = config->pin_pclk,
        .xclk_freq_hz = (int)config->xclk_hz,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = framesize,
        .jpeg_quality = config->jpeg_quality,
        .fb_count = config->native_buffers,
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_LATEST,
    };
    esp_err_t error = esp_camera_init(&native);
    if (error != ESP_OK) {
        ESP_LOGE("CamIo", "Camera initialization failed: %s (0x%x)",
                 esp_err_to_name(error), (unsigned)error);
        return NEO_E_IO;
    }
    backend_initialized = TRUE;
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor == NULL_PTR) {
        /* Core owns rollback and retains the partial-init mutex if it fails. */
        ESP_LOGE("CamIo", "Camera initialized without sensor metadata");
        return NEO_E_IO;
    }
    if (sensor->status.framesize != framesize) {
        ESP_LOGE("CamIo", "Sensor clamped requested camera resolution");
        return NEO_E_PARAM;
    }
    camera_sensor_info_t *sensor_info = esp_camera_sensor_get_info(&sensor->id);
    *info = (Neo_CameraInfoType){
        .width = config->width,
        .height = config->height,
        .sensor_pid = sensor->id.PID,
    };
    (void)snprintf(info->sensor_name, sizeof(info->sensor_name), "%s",
                   sensor_info != NULL_PTR ? sensor_info->name : "Unknown");
    ESP_LOGI("CamIo", "Detected %s PID=0x%x, %ux%u JPEG",
             info->sensor_name, info->sensor_pid, info->width, info->height);
    return NEO_OK;
}

Neo_StatusType CamIo_BackendDeinit(void)
{
    if (!backend_initialized) {
        return NEO_OK;
    }
    esp_err_t error = esp_camera_deinit();
    if (error != ESP_OK) {
        ESP_LOGE("CamIo", "Camera deinitialization failed: %s", esp_err_to_name(error));
        return NEO_E_IO;
    }
    backend_initialized = FALSE;
    return NEO_OK;
}

Neo_StatusType CamIo_BackendAcquire(CamIo_BackendFrameType *frame)
{
    camera_fb_t *native = esp_camera_fb_get();
    if (native == NULL_PTR) {
        return NEO_E_TIMEOUT;
    }
    _Static_assert(SIZE_MAX <= UINT32_MAX, "ESP32 camera sizes must fit project fields");
    if (native->timestamp.tv_sec < 0 || native->timestamp.tv_usec < 0 ||
        native->timestamp.tv_usec >= 1000000) {
        esp_camera_fb_return(native);
        return NEO_E_IO;
    }
    *frame = (CamIo_BackendFrameType){
        .data = native->buf,
        .length = (uint32)native->len,
        .width = (uint32)native->width,
        .height = (uint32)native->height,
        .timestamp_us = (uint64)native->timestamp.tv_sec * 1000000U +
                        (uint64)native->timestamp.tv_usec,
        .jpeg = native->format == PIXFORMAT_JPEG,
        .native_handle = native,
    };
    return NEO_OK;
}

void CamIo_BackendReturn(CamIo_BackendFrameType *frame)
{
    esp_camera_fb_return(frame->native_handle);
    frame->native_handle = NULL_PTR;
}
