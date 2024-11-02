/* date = October 12th 2020 6:33 pm */

#ifndef GC_COMMON_H
#define GC_COMMON_H

#include <stdint.h>
#include <float.h>
#include <assert.h>

#define static_function static // static functions
#define static_persist static // static local variables
#define static_global static // static global variables

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define I8_MIN ((i8)0x80)
#define I16_MIN ((i16)0x8000)
#define I32_MIN ((i32)0x80000000)
#define I64_MIN ((i64)0x8000000000000000LL)

#define U8_MAX 0xffU
#define I8_MAX 0x7f
#define U16_MAX 0xffffU
#define I16_MAX 0x7fff
#define U32_MAX 0xffffffffU
#define I32_MAX 0x7fffffff
#define U64_MAX 0xffffffffffffffffULL
#define I64_MAX 0x7fffffffffffffffLL

#define F32_MAX FLT_MAX
#define F64_MAX DBL_MAX

#define KILOBYTES(value) ((value) * 1024)
#define MEGABYTES(value) (KILOBYTES(value) * 1024)
#define GIGABYTES(value) (MEGABYTES(value) * 1024LL)
#define TERABYTES(value) (GIGABYTES(value) * 1024LL)

#define ARRAYCOUNT(array) (sizeof(array) / sizeof((array)[0]))

#ifdef GC_DEBUG
#define ASSERT(expression) do {if (!(expression)) { *(volatile int *)0 = 0; }} while(0)
#else
#define ASSERT(expression)
#endif // GC_DEBUG

#define ASSERT_RANGE(value, min, max) ASSERT((value) > (min) && (value) < (max))
#define ASSERT_EQ(a, b) ASSERT((a) == (b))

#define ROUND_UP_BY_POWER_OF_2(value, powerOf2) (((value) + (powerOf2) - 1) & ~((powerOf2) - 1))
#define ROUND_DOWN_BY_POWER_OF_2(value, powerOf2) ((value) & ~((powerOf2) - 1))
#define IS_POWER_OF_2(value) (value != 0 && (value & (value - 1)) == 0)

#define MEMBER(type, member) (((type *)0)->member)
#define MEMBER_SIZE(type, member) (sizeof(((type *)0)->member))

#define STRINGISE(token) #token
#define TOKENPASTE_(a, b) a##b
#define TOKENPASTE(a, b) TOKENPASTE_(a, b)

#endif // GC_COMMON_H
