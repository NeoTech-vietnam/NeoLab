#ifndef MEDIA_BUF_H
#define MEDIA_BUF_H

#include "Neo_Types.h"
#include "MediaBuf_Cfg.h"

/* Init/deinit are serialized by BootCtrl after all clients are quiescent.
 * Lease operations are safe between active clients. Acquire gives the writer
 * one reference; after publishing, payloads are immutable. Each successful
 * Retain requires exactly one Release. A generation cannot be resurrected. */
Neo_StatusType MediaBuf_Init(void);
Neo_StatusType MediaBuf_Deinit(void);
Neo_StatusType MediaBuf_Acquire(uint8 **data, Neo_BufferTokenType *token);
Neo_StatusType MediaBuf_AcquirePhoto(uint8 **data, Neo_BufferTokenType *token);
Neo_StatusType MediaBuf_Retain(Neo_BufferTokenType token);
Neo_StatusType MediaBuf_Release(Neo_BufferTokenType token);
uint32 MediaBuf_Capacity(Neo_BufferTokenType token);
uint32 MediaBuf_Outstanding(void);
uint32 MediaBuf_References(void);
uint32 MediaBuf_PreviewReferences(void);

#endif
