#ifndef NEO_HTTP_TYPES_H
#define NEO_HTTP_TYPES_H

#include "Neo_Types.h"

/* Statically connected application ports, independent of HTTP/native handles. */
typedef struct {
    Neo_StatusType (*page)(char *text, uint32 capacity, uint32 *length);
    Neo_StatusType (*metrics)(char *text, uint32 capacity, uint32 *length);
    Neo_StatusType (*frame)(boolean capture, Neo_FrameType *frame);
    Neo_StatusType (*release)(Neo_BufferTokenType token);
    void (*stream_opened)(void);
    void (*stream_frame)(const Neo_FrameType *frame, boolean sent);
} Neo_HttpPortsType;

#endif
