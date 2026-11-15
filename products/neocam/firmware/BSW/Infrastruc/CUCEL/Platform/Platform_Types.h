#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#include <stdint.h>
#include <limits.h>
#include <float.h>

#define CPU_TYPE_8 8U
#define CPU_TYPE_16 16U
#define CPU_TYPE_32 32U
#define CPU_TYPE_64 64U
#define MSB_FIRST 0U
#define LSB_FIRST 1U
#define HIGH_BYTE_FIRST 0U
#define LOW_BYTE_FIRST 1U
#define CPU_TYPE CPU_TYPE_32
#define CPU_BIT_ORDER LSB_FIRST
#define CPU_BYTE_ORDER LOW_BYTE_FIRST

typedef _Bool boolean;
typedef int8_t sint8;
typedef uint8_t uint8;
typedef int16_t sint16;
typedef uint16_t uint16;
typedef int32_t sint32;
typedef uint32_t uint32;
typedef int64_t sint64;
typedef uint64_t uint64;
typedef int_least8_t sint8_least;
typedef uint_least8_t uint8_least;
typedef int_least16_t sint16_least;
typedef uint_least16_t uint16_least;
typedef int_least32_t sint32_least;
typedef uint_least32_t uint32_least;
typedef float float32;
typedef double float64;
typedef void *VoidPtr;
typedef const void *ConstVoidPtr;

#ifndef TRUE
#define TRUE ((boolean)1)
#endif
#ifndef FALSE
#define FALSE ((boolean)0)
#endif
#define UINT8_MIN 0U
#define UINT16_MIN 0U
#define UINT32_MIN 0U
#define UINT64_MIN 0ULL
#define SINT8_MIN INT8_MIN
#define SINT8_MAX INT8_MAX
#define SINT16_MIN INT16_MIN
#define SINT16_MAX INT16_MAX
#define SINT32_MIN INT32_MIN
#define SINT32_MAX INT32_MAX
#define SINT64_MIN INT64_MIN
#define SINT64_MAX INT64_MAX
#define FLOAT32_MIN FLT_MIN
#define FLOAT32_MAX FLT_MAX
#define FLOAT32_EPSILON FLT_EPSILON
#define FLOAT64_MIN DBL_MIN
#define FLOAT64_MAX DBL_MAX
#define FLOAT64_EPSILON DBL_EPSILON

#endif
