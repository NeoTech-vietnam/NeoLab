#ifndef WIFI_MGR_H
#define WIFI_MGR_H

#include "WifiMgr_Cfg.h"

/* Lifecycle is serialized by BootCtrl; readiness may be read concurrently. */
Neo_StatusType WifiMgr_ValidateConfig(const WifiMgr_ConfigType *config);
Neo_StatusType WifiMgr_Init(void);
Neo_StatusType WifiMgr_Start(void);
Neo_StatusType WifiMgr_Stop(void);
Neo_StatusType WifiMgr_Deinit(void);
boolean WifiMgr_IsReady(void);

#endif
