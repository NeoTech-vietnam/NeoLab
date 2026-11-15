#ifndef HTTPSRV_INTERNAL_H
#define HTTPSRV_INTERNAL_H
#include "HttpSrv.h"
Neo_StatusType HttpSrv_BackendStart(const Neo_HttpPortsType *ports);
Neo_StatusType HttpSrv_BackendStop(void);
#endif
