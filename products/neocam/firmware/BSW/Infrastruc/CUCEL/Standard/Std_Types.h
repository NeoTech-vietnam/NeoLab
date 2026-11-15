#ifndef STD_TYPES_H
#define STD_TYPES_H

#include "Platform_Types.h"

typedef uint8 Std_ReturnType;
typedef struct {
    uint16 vendorID;
    uint16 moduleID;
    uint8 sw_major_version;
    uint8 sw_minor_version;
    uint8 sw_patch_version;
} Std_VersionInfoType;

#define E_OK ((Std_ReturnType)0U)
#define E_NOT_OK ((Std_ReturnType)1U)
#define STD_HIGH 1U
#define STD_LOW 0U
#define STD_ACTIVE 1U
#define STD_IDLE 0U
#define STD_ON 1U
#define STD_OFF 0U
#define NULL_PTR ((void *)0)

#endif
