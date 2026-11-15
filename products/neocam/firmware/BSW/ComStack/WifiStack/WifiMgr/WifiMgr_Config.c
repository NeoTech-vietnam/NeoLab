#include "WifiMgr.h"

static uint32 bounded_length(const char *text, uint32 limit)
{
    uint32 length = 0U;
    while (length <= limit && text[length] != '\0') ++length;
    return length;
}

Neo_StatusType WifiMgr_ValidateConfig(const WifiMgr_ConfigType *config)
{
    if (config == NULL_PTR || config->ssid == NULL_PTR || config->password == NULL_PTR) {
        return NEO_E_PARAM;
    }
    uint32 ssid_length = bounded_length(config->ssid, 32U);
    uint32 password_length = bounded_length(config->password, 63U);
    if (ssid_length == 0U || ssid_length > 32U || password_length > 63U ||
        (password_length != 0U && password_length < 8U) ||
        config->channel == 0U || config->channel > 11U ||
        config->max_clients == 0U || config->max_clients > 10U) {
        return NEO_E_PARAM;
    }
    return NEO_OK;
}
