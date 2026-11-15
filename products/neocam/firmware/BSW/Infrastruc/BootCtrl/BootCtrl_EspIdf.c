#include "BootCtrl.h"
#include "esp_log.h"
#include "nvs_flash.h"

static boolean initialized;

Neo_StatusType BootCtrl_PlatformInit(void)
{
    if (initialized) return NEO_OK;
    esp_err_t result = nvs_flash_init();
    if (result != ESP_OK) {
        ESP_LOGE("BootCtrl", "NVS init failed: %s; no partition was erased", esp_err_to_name(result));
        return NEO_E_IO;
    }
    initialized = TRUE;
    return NEO_OK;
}

Neo_StatusType BootCtrl_PlatformDeinit(void)
{
    if (!initialized) return NEO_OK;
    esp_err_t result = nvs_flash_deinit();
    if (result != ESP_OK) {
        ESP_LOGE("BootCtrl", "NVS deinit failed: %s", esp_err_to_name(result));
        return NEO_E_IO;
    }
    initialized = FALSE;
    return NEO_OK;
}
