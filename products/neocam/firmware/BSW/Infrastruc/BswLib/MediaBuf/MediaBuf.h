#ifndef MEDIA_BUF_H
#define MEDIA_BUF_H

#include "Neo_Types.h"
#include "MediaBuf_Cfg.h"

/* Init/deinit are serialized by BootCtrl after all clients are quiescent.
 * Acquire/release/outstanding are safe between active clients. A lease has
 * one owner, remains valid until release, and cannot be released twice. */
Neo_StatusType MediaBuf_Init(void);
Neo_StatusType MediaBuf_Deinit(void);
Neo_StatusType MediaBuf_Acquire(uint8 **data, Neo_BufferTokenType *token);
Neo_StatusType MediaBuf_Release(Neo_BufferTokenType token);
uint32 MediaBuf_Outstanding(void);

#endif
