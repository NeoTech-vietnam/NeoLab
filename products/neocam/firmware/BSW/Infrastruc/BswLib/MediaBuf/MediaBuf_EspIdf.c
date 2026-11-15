#include "MediaBuf_Backend.h"

#include "esp_heap_caps.h"

uint8 *MediaBuf_BackendAllocate(uint32 size)
{
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void MediaBuf_BackendFree(uint8 *data)
{
    heap_caps_free(data);
}
