#include <assert.h>
#include <string.h>

#include "WifiMgr.h"

int main(void)
{
    WifiMgr_ConfigType config = WifiMgr_Config;
    assert(WifiMgr_ValidateConfig(&config) == NEO_OK);
    assert(WifiMgr_ValidateConfig(NULL_PTR) == NEO_E_PARAM);
    config.ssid = NULL_PTR;
    assert(WifiMgr_ValidateConfig(&config) == NEO_E_PARAM);
    config.ssid = "";
    assert(WifiMgr_ValidateConfig(&config) == NEO_E_PARAM);
    char ssid[34];
    memset(ssid, 's', sizeof(ssid));
    ssid[32] = '\0';
    config.ssid = ssid;
    assert(WifiMgr_ValidateConfig(&config) == NEO_OK);
    ssid[32] = 's';
    ssid[33] = '\0';
    assert(WifiMgr_ValidateConfig(&config) == NEO_E_PARAM);
    config.ssid = "camera";
    config.password = "";
    assert(WifiMgr_ValidateConfig(&config) == NEO_OK);
    config.password = "1234567";
    assert(WifiMgr_ValidateConfig(&config) == NEO_E_PARAM);
    config.password = "12345678";
    assert(WifiMgr_ValidateConfig(&config) == NEO_OK);
    char password[65];
    memset(password, 'p', sizeof(password));
    password[63] = '\0';
    config.password = password;
    assert(WifiMgr_ValidateConfig(&config) == NEO_OK);
    password[63] = 'p';
    password[64] = '\0';
    assert(WifiMgr_ValidateConfig(&config) == NEO_E_PARAM);
    config.password = "12345678";
    config.channel = 0U;
    assert(WifiMgr_ValidateConfig(&config) == NEO_E_PARAM);
    config.channel = 12U;
    assert(WifiMgr_ValidateConfig(&config) == NEO_E_PARAM);
    config.channel = 1U;
    config.max_clients = 0U;
    assert(WifiMgr_ValidateConfig(&config) == NEO_E_PARAM);
    config.max_clients = 11U;
    assert(WifiMgr_ValidateConfig(&config) == NEO_E_PARAM);
    return 0;
}
