#include "Neo_CameraTypes.h"
#include <string.h>

#ifndef NEOCAM_BENCH_JPEG_QUALITY
#define NEOCAM_BENCH_JPEG_QUALITY (-1)
#endif
#ifndef NEOCAM_BENCH_SMOOTH_FPS
#define NEOCAM_BENCH_SMOOTH_FPS 30
#endif
_Static_assert(NEOCAM_BENCH_JPEG_QUALITY >= -1 && NEOCAM_BENCH_JPEG_QUALITY <= 63, "JPEG quality range");
#define QUALITY(normal) (NEOCAM_BENCH_JPEG_QUALITY >= 0 ? NEOCAM_BENCH_JPEG_QUALITY : (normal))

/* Performance validation is evidence, not an assumption inferred from settings.
 * Keep each flag FALSE until that exact preset has passed hardware qualification. */
static const Neo_ProfileType profiles[NEO_PROFILE_COUNT] = {
    {NEO_PROFILE_SAFE, "safe", 640, 480, QUALITY(12), 30, FALSE},
    {NEO_PROFILE_SMOOTH, "smooth", 640, 480, QUALITY(16), NEOCAM_BENCH_SMOOTH_FPS, FALSE},
    {NEO_PROFILE_BALANCED, "balanced", 1280, 720, QUALITY(12), 30, FALSE},
    {NEO_PROFILE_DETAIL, "detail", 1600, 1200, QUALITY(10), 10, FALSE},
    {NEO_PROFILE_PHOTO, "photo", 2048, 1536, QUALITY(8), 0, FALSE},
};

const Neo_ProfileType *Neo_ProfileGet(Neo_ProfileIdType id)
{
    return (unsigned)id < NEO_PROFILE_COUNT ? &profiles[id] : NULL_PTR;
}

const Neo_ProfileType *Neo_ProfileFind(const char *name)
{
    if (name == NULL_PTR) return NULL_PTR;
    for (uint32 i = 0; i < NEO_PROFILE_COUNT; ++i)
        if (strcmp(name, profiles[i].name) == 0) return &profiles[i];
    return NULL_PTR;
}
