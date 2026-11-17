#include "CamSupv.h"
#include "Rte.h"
#include <stdio.h>

Neo_StatusType CamSupv_SelectProfile(const char *name)
{
    const Neo_ProfileType *profile = Neo_ProfileFind(name);
    if (profile == NULL_PTR || profile->id == NEO_PROFILE_PHOTO) return NEO_E_PARAM;
    /* The pipeline atomically admits one operation; no duplicate app state machine. */
    return Rte_CameraRequestProfile(profile->id);
}

Neo_StatusType CamSupv_Photo(Neo_FrameType *frame)
{
    if (frame == NULL_PTR) return NEO_E_PARAM;
    return Rte_CameraPhoto(frame);
}

Neo_StatusType CamSupv_ProfileJson(char *text, uint32 capacity, uint32 *length)
{
    if (text == NULL_PTR || capacity == 0 || length == NULL_PTR) return NEO_E_PARAM;
    Neo_PipelineStatsType stats;
    Neo_StatusType result = Rte_CameraStats(&stats);
    if (result != NEO_OK) return result;
    const Neo_ProfileType *active = Neo_ProfileGet(stats.active_profile);
    const Neo_ProfileType *requested = Neo_ProfileGet(stats.requested_profile);
    static const char *states[] = {"stopped", "running", "switching", "photo", "error", "stopping"};
    const char *state = (unsigned)stats.state < sizeof(states)/sizeof(states[0]) ? states[stats.state] : "error";
    int used = snprintf(text, capacity,
        "{\"active\":\"%s\",\"requested\":\"%s\",\"state\":\"%s\",\"epoch\":%lu,"
        "\"last_error\":%u,\"restore_error\":%u,\"profiles\":[",
        active ? active->name : "unknown", requested ? requested->name : "unknown", state,
        (unsigned long)stats.epoch, (unsigned)stats.last_error, (unsigned)stats.restore_error);
    if (used < 0 || (uint32)used >= capacity) return NEO_E_TOO_LARGE;
    for (unsigned i = 0; i < NEO_PROFILE_COUNT; ++i) {
        const Neo_ProfileType *profile = Neo_ProfileGet((Neo_ProfileIdType)i);
        int added = snprintf(text + used, capacity - (uint32)used,
            "%s{\"id\":\"%s\",\"width\":%u,\"height\":%u,\"jpeg_quality\":%u,"
            "\"target_fps\":%u,\"performance_validated\":%s,\"photo_only\":%s}",
            i ? "," : "", profile->name, profile->width, profile->height,
            profile->jpeg_quality, profile->target_fps,
            profile->performance_validated ? "true" : "false",
            profile->id == NEO_PROFILE_PHOTO ? "true" : "false");
        if (added < 0 || (uint32)added >= capacity - (uint32)used) return NEO_E_TOO_LARGE;
        used += added;
    }
    int added = snprintf(text + used, capacity - (uint32)used, "]}");
    if (added < 0 || (uint32)added >= capacity - (uint32)used) return NEO_E_TOO_LARGE;
    *length = (uint32)(used + added);
    return NEO_OK;
}
