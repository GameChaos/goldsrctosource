/* date = October 12th 2020 6:33 pm */

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <float.h>

#define internal static
#define local_persist static
#define global static

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef s32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define S8_MIN ((s8)0x80)
#define S16_MIN ((s16)0x8000)
#define S32_MIN ((s32)0x80000000)
#define S64_MIN ((s64)0x8000000000000000LL)

#define U8_MAX 0xffU
#define S8_MAX 0x7f
#define U16_MAX 0xffffU
#define S16_MAX 0x7fff
#define U32_MAX 0xffffffffU
#define S32_MAX 0x7fffffff
#define U64_MAX 0xffffffffffffffffULL
#define S64_MAX 0x7fffffffffffffffLL

#define F32_PI 3.14159265358979323846f
#define F64_PI 3.14159265358979323846

#define F32_EULER 2.7182818284590452353602874713527f
#define F64_EULER 2.7182818284590452353602874713527

#define F32_MAX FLT_MAX
#define F64_MAX DBL_MAX
#define F32_INFINITY INFINITY
#define F64_INFINITY INFINITY

#define KILOBYTES(value) value * 1024
#define MEGABYTES(value) KILOBYTES(value) * 1024LL
#define GIGABYTES(value) MEGABYTES(value) * 1024LL
#define TERABYTES(value) GIGABYTES(value) * 1024LL

#define TOKENPASTE_(a, b) a##b
#define TOKENPASTE(a, b) TOKENPASTE_(a, b)

#define TOKENPASTE3_(a, b, c) a##b##c
#define TOKENPASTE3(a, b, c) TOKENPASTE3_(a, b, c)

#define ARRAYCOUNT(array) (sizeof(array) / sizeof((array)[0]))

#ifdef GC_DEBUG
#define ASSERT(expression) if (!(expression)) { *(volatile int *)0 = 0; }
#else
#define ASSERT(expression)
#endif

#define MEMBER(type, member) (((type *)0)->member)
#define MEMBER_SIZE(type, member) (sizeof(((type *)0)->member))
#define OFFSETOF(type, member) ((u64)&MEMBER(type, member))

#endif //COMMON_H
