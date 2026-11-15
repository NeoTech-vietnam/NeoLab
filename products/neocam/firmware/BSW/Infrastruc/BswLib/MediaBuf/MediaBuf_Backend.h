#ifndef MEDIA_BUF_BACKEND_H
#define MEDIA_BUF_BACKEND_H

#include "Neo_Types.h"

/* Private allocation seam; allows host ownership tests without an SDK heap. */
uint8 *MediaBuf_BackendAllocate(uint32 size);
void MediaBuf_BackendFree(uint8 *data);

#endif
