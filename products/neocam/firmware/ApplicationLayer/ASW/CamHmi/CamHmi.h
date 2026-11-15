#ifndef CAMHMI_H
#define CAMHMI_H
#include "Neo_HttpTypes.h"
Neo_StatusType CamHmi_Init(void);
Neo_StatusType CamHmi_Deinit(void);
extern const Neo_HttpPortsType CamHmi_HttpPorts;
#endif
