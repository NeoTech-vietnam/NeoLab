/* Module-specific section markers. Deliberately no include guard. */
#if defined(HTTPSRV_START_SEC_CONST_UNSPECIFIED)
#undef HTTPSRV_START_SEC_CONST_UNSPECIFIED
#define NEOCAM_START_SEC_CONST_UNSPECIFIED
#elif defined(HTTPSRV_STOP_SEC_CONST_UNSPECIFIED)
#undef HTTPSRV_STOP_SEC_CONST_UNSPECIFIED
#define NEOCAM_STOP_SEC_CONST_UNSPECIFIED
#else
#error "Unknown HttpSrv section"
#endif
#include "MemMap.h"
