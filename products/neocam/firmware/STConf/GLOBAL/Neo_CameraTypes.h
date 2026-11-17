#ifndef NEO_CAMERA_TYPES_H
#define NEO_CAMERA_TYPES_H
#include "Neo_Types.h"

typedef enum {
    NEO_PROFILE_SAFE = 0,
    NEO_PROFILE_SMOOTH,
    NEO_PROFILE_BALANCED,
    NEO_PROFILE_DETAIL,
    NEO_PROFILE_PHOTO,
    NEO_PROFILE_COUNT
} Neo_ProfileIdType;

typedef struct {
    Neo_ProfileIdType id;
    const char *name;
    uint16 width;
    uint16 height;
    uint8 jpeg_quality;
    uint16 target_fps;
    boolean performance_validated;
} Neo_ProfileType;

typedef enum {
    NEO_PIPE_STOPPED = 0,
    NEO_PIPE_RUNNING,
    NEO_PIPE_SWITCHING,
    NEO_PIPE_PHOTO,
    NEO_PIPE_ERROR,
    NEO_PIPE_STOPPING
} Neo_PipeStateType;

typedef struct {
    Neo_PipeStateType state;
    Neo_ProfileIdType active_profile;
    Neo_ProfileIdType requested_profile;
    uint32 epoch;
    uint64 captured;
    uint64 superseded;
    uint64 failures;
    uint64 oversized;
    uint32 capture_fps_x100;
    uint32 acquire_p95_us;
    uint32 copy_p95_us;
    uint32 min_jpeg_bytes;
    uint32 max_jpeg_bytes;
    uint32 free_psram;
    uint32 largest_psram;
    uint32 free_internal;
    uint32 min_free_internal;
    uint32 producer_stack_bytes;
    uint32 cpu_mhz;
    uint32 psram_mhz;
    boolean psram_dma;
    uint32 min_free_psram;
    uint32 pool_outstanding;
    uint32 pool_references;
    uint32 last_operation_us;
    uint32 last_photo_pause_us;
    uint8 wifi_channel;
    uint8 wifi_bandwidth_mhz;
    Neo_StatusType last_error;
    Neo_StatusType restore_error;
} Neo_PipelineStatsType;

const Neo_ProfileType *Neo_ProfileGet(Neo_ProfileIdType id);
const Neo_ProfileType *Neo_ProfileFind(const char *name);

#endif
