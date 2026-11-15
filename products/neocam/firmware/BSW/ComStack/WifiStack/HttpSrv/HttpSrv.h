#ifndef HTTPSRV_H
#define HTTPSRV_H

#include "Neo_HttpTypes.h"

Neo_StatusType HttpSrv_Start(const Neo_HttpPortsType *ports);
Neo_StatusType HttpSrv_Stop(void);

#endif
