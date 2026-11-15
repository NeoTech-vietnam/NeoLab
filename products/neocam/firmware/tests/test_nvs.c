#include <assert.h>
#include "BootCtrl.h"
#include "nvs_flash.h"

static esp_err_t init_result, deinit_result;
static unsigned starts, stops;
esp_err_t nvs_flash_init(void) { ++starts; return init_result; }
esp_err_t nvs_flash_deinit(void) { ++stops; return deinit_result; }
const char *esp_err_to_name(esp_err_t error) { (void)error; return "injected"; }
void fake_http_log(const char *tag, const char *format, ...) { assert(tag && format); }

int main(void)
{
    assert(BootCtrl_PlatformDeinit() == NEO_OK && stops == 0);
    init_result = ESP_ERR_NVS_NO_FREE_PAGES;
    assert(BootCtrl_PlatformInit() == NEO_E_IO);
    assert(BootCtrl_PlatformDeinit() == NEO_OK && stops == 0);
    init_result = ESP_ERR_NVS_NEW_VERSION_FOUND;
    assert(BootCtrl_PlatformInit() == NEO_E_IO);
    init_result = ESP_OK;
    assert(BootCtrl_PlatformInit() == NEO_OK);
    assert(BootCtrl_PlatformInit() == NEO_OK && starts == 3);
    deinit_result = 3;
    assert(BootCtrl_PlatformDeinit() == NEO_E_IO);
    assert(BootCtrl_PlatformInit() == NEO_OK && starts == 3);
    deinit_result = ESP_OK;
    assert(BootCtrl_PlatformDeinit() == NEO_OK);
    assert(BootCtrl_PlatformDeinit() == NEO_OK && stops == 2);
    return 0;
}
