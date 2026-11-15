#ifndef HTTPSRV_CFG_H
#define HTTPSRV_CFG_H
#include "Std_Types.h"

typedef struct {
    uint16 port;
    uint16 socket_timeout_s;
    uint32 worker_stack_bytes;
    uint32 stop_timeout_ms;
} HttpSrv_ConfigType;

extern const HttpSrv_ConfigType HttpSrv_Config;
#endif
