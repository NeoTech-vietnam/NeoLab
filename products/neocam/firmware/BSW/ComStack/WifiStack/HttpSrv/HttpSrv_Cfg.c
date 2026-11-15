#include "HttpSrv_Cfg.h"

#define HTTPSRV_START_SEC_CONST_UNSPECIFIED
#include "HttpSrv_MemMap.h"
NEOCAM_SECTION const HttpSrv_ConfigType HttpSrv_Config = {
    .port = 80,
    .socket_timeout_s = 2,
    .worker_stack_bytes = 6144,
    .stop_timeout_ms = 10000
};
#define HTTPSRV_STOP_SEC_CONST_UNSPECIFIED
#include "HttpSrv_MemMap.h"
