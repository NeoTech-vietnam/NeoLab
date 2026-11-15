#ifndef BOOTCTRL_H
#define BOOTCTRL_H

#include "Neo_Types.h"

typedef struct {
    const char *name;
    Neo_StatusType (*start)(void);
    Neo_StatusType (*stop)(void);
} BootCtrl_StepType;

typedef struct {
    const BootCtrl_StepType *steps;
    uint32 count;
} BootCtrl_ConfigType;

typedef struct {
    Neo_StatusType primary;
    Neo_StatusType cleanup;
    const char *stage;
    uint32 active_steps;
} BootCtrl_StatusType;

/* Lifecycle calls are serialized by the composition owner, never from ISR. */
Neo_StatusType BootCtrl_Start(const BootCtrl_ConfigType *config);
Neo_StatusType BootCtrl_Stop(void);
BootCtrl_StatusType BootCtrl_GetStatus(void);
Neo_StatusType BootCtrl_PlatformInit(void);
Neo_StatusType BootCtrl_PlatformDeinit(void);

#endif
