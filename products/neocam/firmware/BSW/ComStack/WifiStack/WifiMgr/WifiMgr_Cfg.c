#include "WifiMgr_Cfg.h"

const WifiMgr_ConfigType WifiMgr_Config = {
    .ssid = "XIAO-Camera",
    .password = "xiao-camera",
    .channel = NEOCAM_WIFI_CHANNEL,
    .max_clients = 2U,
    .bandwidth_mhz = NEOCAM_WIFI_BANDWIDTH,
};
