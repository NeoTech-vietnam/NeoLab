#ifndef NEO_TYPES_H
#define NEO_TYPES_H

#include "Std_Types.h"

/* Project extensions; these do not extend AUTOSAR Platform_Types.h. */
typedef enum {
    NEO_OK = 0,
    NEO_E_PARAM,
    NEO_E_STATE,
    NEO_E_BUSY,
    NEO_E_MEMORY,
    NEO_E_IO,
    NEO_E_TIMEOUT,
    NEO_E_TOO_LARGE,
    NEO_E_NOT_READY
} Neo_StatusType;

typedef struct {
    uint16 slot;
    uint32 generation;
} Neo_BufferTokenType;

typedef struct {
    const uint8 *data;
    uint32 length;
    uint16 width;
    uint16 height;
    uint32 capture_ms;
    uint64 timestamp_us;
    Neo_BufferTokenType token;
    uint64 sequence;
    uint32 profile_epoch;
    uint16 target_fps;
    uint32 copy_us;
    uint32 acquire_us;
} Neo_FrameType;

typedef struct {
    uint16 width;
    uint16 height;
    uint16 sensor_pid;
    char sensor_name[16];
} Neo_CameraInfoType;

#endif
