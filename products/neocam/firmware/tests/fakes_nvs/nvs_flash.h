#ifndef FAKE_NVS_FLASH_H
#define FAKE_NVS_FLASH_H
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_NVS_NO_FREE_PAGES 1
#define ESP_ERR_NVS_NEW_VERSION_FOUND 2
esp_err_t nvs_flash_init(void);
esp_err_t nvs_flash_deinit(void);
const char *esp_err_to_name(esp_err_t error);
#endif
