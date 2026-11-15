#ifndef WIFI_IF_H
#define WIFI_IF_H

#include "WifiMgr_Cfg.h"

/* Project adapter, not the AUTOSAR 802.11p WEth driver. */
Neo_StatusType WifiIf_Init(const WifiMgr_ConfigType *config);
Neo_StatusType WifiIf_Start(void);
Neo_StatusType WifiIf_Stop(void);
Neo_StatusType WifiIf_Deinit(void);
boolean WifiIf_IsReady(void);
sint32 WifiIf_GetLastNativeError(void);

#endif
