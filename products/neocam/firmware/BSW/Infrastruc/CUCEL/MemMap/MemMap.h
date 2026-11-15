/* Deliberately no include guard: section transitions are included repeatedly. */
#include "NeoCam_Sections.h"

#if defined(NEOCAM_START_SEC_CONST_UNSPECIFIED)
#undef NEOCAM_START_SEC_CONST_UNSPECIFIED
#ifdef NEOCAM_SECTION_OPEN
#error "Nested NeoCam memory section"
#endif
#define NEOCAM_SECTION_OPEN
#define NEOCAM_CONST_OPEN
#define NEOCAM_SECTION NEOCAM_CONST_SECTION
#elif defined(NEOCAM_STOP_SEC_CONST_UNSPECIFIED)
#undef NEOCAM_STOP_SEC_CONST_UNSPECIFIED
#ifndef NEOCAM_CONST_OPEN
#error "NeoCam const memory section was not opened"
#endif
#undef NEOCAM_SECTION_OPEN
#undef NEOCAM_CONST_OPEN
#undef NEOCAM_SECTION
#elif defined(NEOCAM_START_SEC_VAR_CLEARED_UNSPECIFIED)
#undef NEOCAM_START_SEC_VAR_CLEARED_UNSPECIFIED
#ifdef NEOCAM_SECTION_OPEN
#error "Nested NeoCam memory section"
#endif
#define NEOCAM_SECTION_OPEN
#define NEOCAM_ZERO_OPEN
#define NEOCAM_SECTION NEOCAM_ZERO_SECTION
#elif defined(NEOCAM_STOP_SEC_VAR_CLEARED_UNSPECIFIED)
#undef NEOCAM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#ifndef NEOCAM_ZERO_OPEN
#error "NeoCam zero memory section was not opened"
#endif
#undef NEOCAM_SECTION_OPEN
#undef NEOCAM_ZERO_OPEN
#undef NEOCAM_SECTION
#else
#error "Unknown NeoCam memory-section marker"
#endif
