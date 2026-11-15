#ifndef WIFI_MGR_CFG_H
#define WIFI_MGR_CFG_H

#include "Neo_Types.h"

#define WIFI_MGR_STATE_TIMEOUT_MS 5000U

typedef struct {
    const char *ssid;
    const char *password;
    uint8 channel;
    uint8 max_clients;
} WifiMgr_ConfigType;

extern const WifiMgr_ConfigType WifiMgr_Config;

#endif
