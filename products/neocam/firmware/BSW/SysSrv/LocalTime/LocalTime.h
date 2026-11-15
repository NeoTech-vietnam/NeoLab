#ifndef LOCAL_TIME_H
#define LOCAL_TIME_H

#include "Neo_Types.h"

/* Monotonic microseconds since boot; this is not wall/network time. */
uint64 LocalTime_GetUs(void);

#endif
