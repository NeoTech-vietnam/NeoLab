#ifndef EBOS_H
#define EBOS_H

#include "Neo_Types.h"

#define EBOS_WAIT_FOREVER UINT32_MAX

typedef struct EbOs_Mutex *EbOs_MutexType;

/* Creation/deletion require quiescent users; lifecycle calls are serialized. */
Neo_StatusType EbOs_MutexCreate(EbOs_MutexType *out);
Neo_StatusType EbOs_MutexLock(EbOs_MutexType mutex, uint32 timeout_ms);
void EbOs_MutexUnlock(EbOs_MutexType mutex);
void EbOs_MutexDelete(EbOs_MutexType mutex);
void EbOs_Delay(uint32 ms);

#endif
