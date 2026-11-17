#include "WifiIf.h"
#include "WifiMgr.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"

static esp_netif_t *ap_netif;
static esp_event_handler_instance_t event_handler;
static boolean netif_initialized;
static boolean owns_event_loop;
static boolean wifi_initialized;
static boolean wifi_started;
static boolean netif_attached;
static atomic_bool ready;
static esp_err_t first_native_error;

static Neo_StatusType record_error(esp_err_t error, const char *operation)
{
    if (error == ESP_OK) return NEO_OK;
    if (first_native_error == ESP_OK) first_native_error = error;
    ESP_LOGE("WifiIf", "%s: %s", operation, esp_err_to_name(error));
    return error == ESP_ERR_NO_MEM ? NEO_E_MEMORY : NEO_E_IO;
}

static void wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    (void)base;
    (void)data;
    if (id == WIFI_EVENT_AP_START) atomic_store(&ready, true);
    if (id == WIFI_EVENT_AP_STOP) atomic_store(&ready, false);
}

Neo_StatusType WifiIf_Init(const WifiMgr_ConfigType *config)
{
    if (WifiMgr_ValidateConfig(config) != NEO_OK) return NEO_E_PARAM;
    if (wifi_initialized || ap_netif != NULL_PTR || owns_event_loop || event_handler != NULL_PTR) {
        return NEO_E_STATE;
    }
    first_native_error = ESP_OK;
    esp_err_t error;
    if (!netif_initialized) {
        error = esp_netif_init();
        if (error != ESP_OK) return record_error(error, "netif init");
        /* ESP-IDF cannot deinitialize its TCP/IP stack; retain it across restarts. */
        netif_initialized = TRUE;
    }
    error = esp_event_loop_create_default();
    if (error == ESP_OK) owns_event_loop = TRUE;
    else if (error != ESP_ERR_INVALID_STATE) return record_error(error, "event loop init");

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_WIFI_AP();
    ap_netif = esp_netif_new(&netif_config);
    if (ap_netif == NULL_PTR) {
        error = ESP_ERR_NO_MEM;
        goto failed;
    }
    error = esp_netif_attach_wifi_ap(ap_netif);
    if (error != ESP_OK) goto failed;
    netif_attached = TRUE;
    error = esp_wifi_set_default_wifi_ap_handlers();
    if (error != ESP_OK) goto failed;
    error = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event,
                                               NULL_PTR, &event_handler);
    if (error != ESP_OK) goto failed;

    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
    error = esp_wifi_init(&init);
    if (error != ESP_OK) goto failed;
    wifi_initialized = TRUE;
    error = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (error != ESP_OK) goto failed;
    wifi_config_t wifi = {0};
    size_t ssid_length = strlen(config->ssid);
    memcpy(wifi.ap.ssid, config->ssid, ssid_length);
    memcpy(wifi.ap.password, config->password, strlen(config->password));
    wifi.ap.ssid_len = ssid_length;
    wifi.ap.channel = config->channel;
    wifi.ap.max_connection = config->max_clients;
    wifi.ap.authmode = config->password[0] == '\0' ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;
    error = esp_wifi_set_mode(WIFI_MODE_AP);
    if (error == ESP_OK) error = esp_wifi_set_bandwidth(WIFI_IF_AP,
        config->bandwidth_mhz == 40U ? WIFI_BW40 : WIFI_BW20);
    if (error == ESP_OK) error = esp_wifi_set_config(WIFI_IF_AP, &wifi);
    if (error != ESP_OK) goto failed;
    return NEO_OK;

failed:
    {
        Neo_StatusType result = record_error(error, "AP initialization");
        (void)WifiIf_Deinit();
        return result;
    }
}

Neo_StatusType WifiIf_Start(void)
{
    if (!wifi_initialized) return NEO_E_STATE;
    if (wifi_started) return NEO_OK;
    esp_err_t error = esp_wifi_start();
    if (error != ESP_OK) return record_error(error, "AP start");
    wifi_started = TRUE;
    return NEO_OK;
}

Neo_StatusType WifiIf_Stop(void)
{
    if (!wifi_started) return NEO_OK;
    esp_err_t error = esp_wifi_stop();
    if (error != ESP_OK) return record_error(error, "AP stop");
    wifi_started = FALSE;
    return NEO_OK;
}

Neo_StatusType WifiIf_Deinit(void)
{
    Neo_StatusType result = WifiIf_Stop();
    if (result != NEO_OK) return result;
    if (wifi_initialized) {
        esp_err_t error = esp_wifi_deinit();
        if (error != ESP_OK) return record_error(error, "Wi-Fi deinit");
        wifi_initialized = FALSE;
    }
    if (event_handler != NULL_PTR) {
        esp_err_t error = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler);
        if (error != ESP_OK) return record_error(error, "event unregister");
        event_handler = NULL_PTR;
    }
    if (ap_netif != NULL_PTR) {
        if (netif_attached) {
            esp_err_t error = esp_wifi_clear_default_wifi_driver_and_handlers(ap_netif);
            if (error != ESP_OK) return record_error(error, "netif detach");
            netif_attached = FALSE;
        }
        esp_netif_destroy(ap_netif);
        ap_netif = NULL_PTR;
    }
    if (owns_event_loop) {
        esp_err_t error = esp_event_loop_delete_default();
        if (error != ESP_OK) return record_error(error, "event loop deinit");
        owns_event_loop = FALSE;
    }
    atomic_store(&ready, false);
    return NEO_OK;
}

boolean WifiIf_IsReady(void)
{
    return atomic_load(&ready);
}

sint32 WifiIf_GetLastNativeError(void)
{
    return (sint32)first_native_error;
}
