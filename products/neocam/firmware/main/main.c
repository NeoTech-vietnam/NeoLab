#include "NeoCam_Composition.h"
#include "esp_log.h"

void app_main(void)
{
    Neo_StatusType result = NeoCam_Start();
    if (result != NEO_OK) {
        BootCtrl_StatusType status = BootCtrl_GetStatus();
        ESP_LOGE("NeoCam", "Startup failed at %s: %d; cleanup=%d; active=%lu",
                 status.stage != NULL_PTR ? status.stage : "configuration", result,
                 status.cleanup, (unsigned long)status.active_steps);
        return;
    }
    ESP_LOGI("NeoCam", "Ready: connect to the configured SoftAP and open http://192.168.4.1/");
}
