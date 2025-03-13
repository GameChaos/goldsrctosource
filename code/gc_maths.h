/* date = October 12th 2022 3:31 pm */

#ifndef GC_MATHS_H
#define GC_MATHS_H

#define GCM_PI 3.141592653589793238462643383279502884197169399f
#define GCM_PI_2 1.570796326794896619231321691639751442098584699f
#define GCM_PI_4 0.7853981633974483096156608458198757210492923f

#define GCM_PI64 3.141592653589793238462643383279502884197169399
#define GCM_PI64_2 1.570796326794896619231321691639751442098584699
#define GCM_PI64_4 0.7853981633974483096156608458198757210492923

#define GCM_EULER 2.7182818284590452353602874713527f
#define GCM_EULER64 2.7182818284590452353602874713527

#define GCM_RADTODEG(value) ((value) * 180.0f / GCM_PI)
#define GCM_DEGTORAD(value) ((value) * GCM_PI / 180.0f)

#define GCM_MIN(a, b) ((a) > (b) ? (b) : (a))
#define GCM_MAX(a, b) ((a) < (b) ? (b) : (a))
#define GCM_ABS(value) ((value) < 0 ? -(value) : (value))
#define GCM_CLAMP(min, val, max) (GCM_MAX(GCM_MIN(val, max), min))

#include <stdalign.h>
#include <stdint.h>

typedef union v2i {
	struct {
		int32_t x, y;
	};
	struct {
		int32_t u, v;
	};
	int32_t e[2];
} v2i;

typedef union v3i {
	struct {
		int32_t x, y, z;
	};
	struct {
		int32_t r, g, b;
	};
	int32_t e[3];
} v3i;

typedef union v4i {
	struct {
		int32_t x, y, z, w;
	};
	struct {
		int32_t r, g, b, a;
	};
	struct {
		v3i xyz;
	};
	struct {
		v3i rgb;
	};
	int32_t e[4];
} v4i;

typedef union v2 {
	struct {
		float x, y;
	};
	struct {
		float u, v;
	};
	float e[2];
} v2;

typedef union v3 {
	struct {
		float x, y, z;
	};
	struct {
		float r, g, b;
	};
	struct {
		float pitch, yaw, roll;
	};
	v2 xy;
	float e[3];
} v3;

typedef union v4 {
	struct {
		float x, y, z, w;
	};
	struct {
		float r, g, b, a;
	};
	v2 xy;
	v3 xyz;
	v3 rgb;
	float e[4];
} v4;

typedef union mat3 {
	float e[3][3];
	float arr[9];
	v3 col[3]; // column
} mat3;

typedef union mat4 {
	float e[4][4];
	float arr[16];
	v4 col[4]; // column
} mat4;

#ifndef gcm_bool
#include <stdbool.h>
typedef bool gcm_bool;
#endif

#ifndef gcm_func
#define gcm_func static
#endif

gcm_func v2 v2add(v2 a, v2 b);
gcm_func v2 v2adds(v2 a, float b);
gcm_func v2 v2sub(v2 a, v2 b);
gcm_func v2 v2subs(v2 a, float b);
gcm_func v2 v2mul(v2 a, v2 b);
gcm_func v2 v2muls(v2 a, float b);
gcm_func v2 v2div(v2 a, v2 b);
gcm_func v2 v2divs(v2 a, float b);
gcm_func float v2dot(v2 a, v2 b);
gcm_func float v2cross(v2 a, v2 b);
gcm_func v2 v2mod(v2 a, v2 b);
gcm_func v2 v2mods(v2 a, float b);
gcm_func v2 v2max(v2 a, v2 b);
gcm_func v2 v2min(v2 a, v2 b);
gcm_func gcm_bool v2equals(v2 a, v2 b);
gcm_func v2 v2sign(v2 value);
gcm_func v2 v2abs(v2 value);
gcm_func v2i v2tov2i(v2 value);
gcm_func v2 v2fill(float value);

gcm_func v2 v2floor(v2 value);
gcm_func v2 v2ceil(v2 value);
gcm_func v2 v2round(v2 value);
gcm_func v2 v2negate(v2 value);
gcm_func v2 v2normalise(v2 value);
gcm_func float v2normInPlace(v2 *value);
gcm_func float v2lensq(v2 value);
gcm_func float v2len(v2 value);
gcm_func v2 v2lerp(v2 a, v2 b, float t);


gcm_func v3 v3add(v3 a, v3 b);
gcm_func v3 v3adds(v3 a, float b);
gcm_func v3 v3sub(v3 a, v3 b);
gcm_func v3 v3subs(v3 a, float b);
gcm_func v3 v3mul(v3 a, v3 b);
gcm_func v3 v3muls(v3 a, float b);
gcm_func v3 v3div(v3 a, v3 b);
gcm_func v3 v3divs(v3 a, float b);
gcm_func float v3dot(v3 a, v3 b);
gcm_func v3 v3cross(v3 a, v3 b);
gcm_func v3 v3mod(v3 a, v3 b);
gcm_func v3 v3mods(v3 a, float b);
gcm_func v3 v3max(v3 a, v3 b);
gcm_func v3 v3min(v3 a, v3 b);
gcm_func gcm_bool v3equals(v3 a, v3 b);
gcm_func v3 v3sign(v3 value);
gcm_func v3 v3abs(v3 value);
gcm_func v3i v3tov3i(v3 value);
gcm_func v3 v3fill(float value);

gcm_func v3 v3floor(v3 value);
gcm_func v3 v3ceil(v3 value);
gcm_func v3 v3round(v3 value);
gcm_func v3 v3negate(v3 value);
gcm_func v3 v3normalise(v3 value);
gcm_func float v3normInPlace(v3 *value);
gcm_func float v3lensq(v3 value);
gcm_func float v3len(v3 value);
gcm_func v3 v3lerp(v3 a, v3 b, float t);

gcm_func v4 v4add(v4 a, v4 b);
gcm_func v4 v4adds(v4 a, float b);
gcm_func v4 v4sub(v4 a, v4 b);
gcm_func v4 v4subs(v4 a, float b);
gcm_func v4 v4mul(v4 a, v4 b);
gcm_func v4 v4muls(v4 a, float b);
gcm_func v4 v4div(v4 a, v4 b);
gcm_func v4 v4divs(v4 a, float b);
gcm_func float v4dot(v4 a, v4 b);
// Cross product doesn't exactly exist in 4 dimensions.
gcm_func v4 v4mod(v4 a, v4 b);
gcm_func v4 v4mods(v4 a, float b);
gcm_func v4 v4max(v4 a, v4 b);
gcm_func v4 v4min(v4 a, v4 b);
gcm_func gcm_bool v4equals(v4 a, v4 b);
gcm_func v4 v4sign(v4 value);
gcm_func v4 v4abs(v4 value);
gcm_func v4i v4tov4i(v4 value);
gcm_func v4 v4fill(float value);
gcm_func v4 v4fromv3(v3 xyz, float w);

gcm_func v4 v4floor(v4 value);
gcm_func v4 v4ceil(v4 value);
gcm_func v4 v4round(v4 value);
gcm_func v4 v4negate(v4 value);
gcm_func v4 v4normalise(v4 value);
gcm_func float v4normInPlace(v4 *value);
gcm_func float v4lensq(v4 value);
gcm_func float v4len(v4 value);
gcm_func v4 v4lerp(v4 a, v4 b, float t);


gcm_func v2i v2iadd(v2i a, v2i b);
gcm_func v2i v2iadds(v2i a, int32_t b);
gcm_func v2i v2isub(v2i a, v2i b);
gcm_func v2i v2isubs(v2i a, int32_t b);
gcm_func v2i v2imul(v2i a, v2i b);
gcm_func v2i v2imuls(v2i a, int32_t b);
gcm_func v2i v2idiv(v2i a, v2i b);
gcm_func v2i v2idivs(v2i a, int32_t b);
gcm_func v2i v2imod(v2i a, v2i b);
gcm_func v2i v2imods(v2i a, int32_t b);
gcm_func v2i v2imax(v2i a, v2i b);
gcm_func v2i v2imin(v2i a, v2i b);
gcm_func gcm_bool v2iequals(v2i a, v2i b);
gcm_func v2i v2isign(v2i value);
gcm_func v2i v2iabs(v2i value);
gcm_func v2 v2itov2(v2i value);
gcm_func v2i v2ifill(int32_t value);

gcm_func v2i v2idivfloor(v2i a, v2i b);
gcm_func v2i v2idivfloors(v2i a, int32_t b);
gcm_func v2i v2idivceil(v2i a, v2i b);
gcm_func v2i v2idivceils(v2i a, int32_t b);
gcm_func v2i v2inegate(v2i value);


gcm_func v3i v3iadd(v3i a, v3i b);
gcm_func v3i v3iadds(v3i a, int32_t b);
gcm_func v3i v3isub(v3i a, v3i b);
gcm_func v3i v3isubs(v3i a, int32_t b);
gcm_func v3i v3imul(v3i a, v3i b);
gcm_func v3i v3imuls(v3i a, int32_t b);
gcm_func v3i v3idiv(v3i a, v3i b);
gcm_func v3i v3idivs(v3i a, int32_t b);
gcm_func v3i v3imod(v3i a, v3i b);
gcm_func v3i v3imods(v3i a, int32_t b);
gcm_func v3i v3imax(v3i a, v3i b);
gcm_func v3i v3imin(v3i a, v3i b);
gcm_func gcm_bool v3iequals(v3i a, v3i b);
gcm_func v3i v3isign(v3i value);
gcm_func v3i v3iabs(v3i value);
gcm_func v3 v3itov3(v3i value);
gcm_func v3i v3ifill(int32_t value);

gcm_func v3i v3idivfloor(v3i a, v3i b);
gcm_func v3i v3idivfloors(v3i a, int32_t b);
gcm_func v3i v3idivceil(v3i a, v3i b);
gcm_func v3i v3idivceils(v3i a, int32_t b);
gcm_func v3i v3inegate(v3i value);


gcm_func v4i v4iadd(v4i a, v4i b);
gcm_func v4i v4iadds(v4i a, int32_t b);
gcm_func v4i v4isub(v4i a, v4i b);
gcm_func v4i v4isubs(v4i a, int32_t b);
gcm_func v4i v4imul(v4i a, v4i b);
gcm_func v4i v4imuls(v4i a, int32_t b);
gcm_func v4i v4idiv(v4i a, v4i b);
gcm_func v4i v4idivs(v4i a, int32_t b);
gcm_func v4i v4imod(v4i a, v4i b);
gcm_func v4i v4imods(v4i a, int32_t b);
gcm_func v4i v4imax(v4i a, v4i b);
gcm_func v4i v4imin(v4i a, v4i b);
gcm_func gcm_bool v4iequals(v4i a, v4i b);
gcm_func v4i v4isign(v4i value);
gcm_func v4i v4iabs(v4i value);
gcm_func v4 v4itov4(v4i value);
gcm_func v4i v4ifill(int32_t value);

gcm_func v4i v4idivfloor(v4i a, v4i b);
gcm_func v4i v4idivfloors(v4i a, int32_t b);
gcm_func v4i v4idivceil(v4i a, v4i b);
gcm_func v4i v4idivceils(v4i a, int32_t b);
gcm_func v4i v4inegate(v4i value);


gcm_func int32_t i32wrap(int32_t value, int32_t max);
gcm_func int32_t i32divfloor(int32_t a, int32_t b);
gcm_func int32_t i32divceil(int32_t a, int32_t b);
gcm_func int32_t i32mod(int32_t a, int32_t b);
gcm_func int32_t i32sign(int32_t value);


gcm_func float f32cos(float value);
gcm_func float f32sin(float value);
gcm_func float f32exp(float value);
gcm_func float f32log(float value);
gcm_func float f32log2(float value);
gcm_func float f32sqrt(float value);
gcm_func float f32tan(float value);
gcm_func float f32atan(float value);
gcm_func float f32atan2(float y, float x);
gcm_func float f32asin(float value);
gcm_func float f32acos(float value);
gcm_func float f32pow(float a, float b);
gcm_func float f32mod(float a, float b);
gcm_func float f32sign(float value);
gcm_func float f32round(float value);
gcm_func float f32floor(float value);
gcm_func float f32ceil(float value);

gcm_func double f64cos(double value);
gcm_func double f64sin(double value);
gcm_func double f64exp(double value);
gcm_func double f64log(double value);
gcm_func double f64log2(double value);
gcm_func double f64sqrt(double value);
gcm_func double f64tan(double value);
gcm_func double f64atan(double value);
gcm_func double f64atan2(double y, double x);
gcm_func double f64asin(double value);
gcm_func double f64acos(double value);
gcm_func double f64pow(double a, double b);
gcm_func double f64mod(double a, double b);
gcm_func double f64sign(double value);
gcm_func double f64round(double value);
gcm_func double f64floor(double value);
gcm_func double f64ceil(double value);

gcm_func float f32lerp(float a, float b, float t);
gcm_func void AnglesToVectors(v3 angles, v3 *forwards, v3 *right, v3 *up);
gcm_func v3 VectorToAngles(v3 vector);

gcm_func mat4 mat4transpose(mat4 matrix);
gcm_func mat4 mat4diagonal(float diagonal);
gcm_func mat4 mat4invert(mat4 in);
gcm_func mat4 mat4rotation(v3 eulerAngles);
gcm_func mat4 mat4translation(v3 translation);
gcm_func mat4 mat4scale(v3 scale);
gcm_func mat4 mat4scalef32(float scale);
gcm_func mat4 mat4mul(mat4 left, mat4 right);
gcm_func v4 mat4mulv4(mat4 matrix, v4 vec);

gcm_func mat3 mat3transpose(mat3 matrix);
gcm_func mat3 mat3diagonal(float diagonal);
gcm_func mat3 mat3invert(mat3 matrix);
gcm_func mat3 mat3translation2d(v2 translation); // 2d translation
gcm_func mat3 mat3scale(v3 scale);
gcm_func mat3 mat3scale2d(v2 scale);
gcm_func mat3 mat3scalef32(float scale);
gcm_func mat3 mat3mul(mat3 left, mat3 right);
gcm_func v3 mat3mulv3(mat3 matrix, v3 vec);
gcm_func mat3 mat3rotate(float angle, v3 axis);
gcm_func mat4 mat3tomat4(mat3 matrix);

gcm_func uint64_t NextPowerOf2(uint64_t value);

#endif // GC_MATHS_H

#ifdef GC_MATHS_IMPLEMENTATION

#include "smmintrin.h"
#include <math.h>

typedef union simd128 {
	__m128 packedF;
	__m128i packedI;
	float arrayF32[4];
	int32_t arrayS32[4];
	v4 vec4;
} simd128;

gcm_func v2 v2add(v2 a, v2 b) {
	v2 result = {a.x + b.x, a.y + b.y};
	return result;
}

gcm_func v2 v2adds(v2 a, float b) {
	v2 result = {a.x + b, a.y + b};
	return result;
}

gcm_func v2 v2sub(v2 a, v2 b) {
	v2 result = {a.x - b.x, a.y - b.y};
	return result;
}

gcm_func v2 v2subs(v2 a, float b) {
	v2 result = {a.x - b, a.y - b};
	return result;
}

gcm_func v2 v2mul(v2 a, v2 b)
{
	v2 result = {a.x * b.x, a.y * b.y};
	return result;
}

gcm_func v2 v2muls(v2 a, float b) {
	v2 result = {a.x * b, a.y * b};
	return result;
}

gcm_func v2 v2div(v2 a, v2 b) {
	v2 result = {a.x / b.x, a.y / b.y};
	return result;
}

gcm_func v2 v2divs(v2 a, float b) {
	v2 result = {a.x / b, a.y / b};
	return result;
}

gcm_func float v2dot(v2 a, v2 b) {
	float result = a.x * b.x + a.y * b.y;
	return result;
}

gcm_func float v2cross(v2 a, v2 b) {
	float result = (a.x * b.y) - (a.y * b.x);
	return result;
}

gcm_func v2 v2mod(v2 a, v2 b) {
	v2 result = {f32mod(a.x, b.x), f32mod(a.y, b.y)};
	return result;
}

gcm_func v2 v2mods(v2 a, float b) {
	v2 result = {f32mod(a.x, b), f32mod(a.y, b)};
	return result;
}

gcm_func v2 v2max(v2 a, v2 b) {
	v2 result = {GCM_MAX(a.x, b.x), GCM_MAX(a.y, b.y)};
	return result;
}

gcm_func v2 v2min(v2 a, v2 b) {
	v2 result = {GCM_MIN(a.x, b.x), GCM_MIN(a.y, b.y)};
	return result;
}

gcm_func gcm_bool v2equals(v2 a, v2 b) {
	gcm_bool result = (a.x == b.x && a.y == b.y);
	return result;
}

gcm_func v2 v2sign(v2 value) {
	v2 result = {f32sign(value.x), f32sign(value.y)};
	return result;
}

gcm_func v2 v2abs(v2 value) {
	v2 result = {GCM_ABS(value.x), GCM_ABS(value.y)};
	return result;
}

gcm_func v2i v2tov2i(v2 value) {
	v2i result = {(int32_t)value.x, (int32_t)value.y};
	return result;
}

gcm_func v2 v2fill(float value) {
	v2 result = {value, value};
	return result;
}

gcm_func v2 v2floor(v2 value) {
	v2 result = {f32floor(value.x), f32floor(value.y)};
	return result;
}

gcm_func v2 v2ceil(v2 value) {
	v2 result = {f32ceil(value.x), f32ceil(value.y)};
	return result;
}

gcm_func v2 v2round(v2 value) {
	v2 result = {f32round(value.x), f32round(value.y)};
	return result;
}

gcm_func v2 v2negate(v2 value) {
	v2 result = {-value.x, -value.y};
	return result;
}

gcm_func v2 v2normalise(v2 value) {
	float length = v2len(value);
	if (length > 0)
	{
		length = 1.0f / length;
	}
	v2 result = v2muls(value, length);
	return result;
}

gcm_func float v2normInPlace(v2 *value) {
	float result = v2len(*value);
	float invLength = result;
	if (invLength > 0)
	{
		invLength = 1.0f / invLength;
	}
	*value = v2muls(*value, invLength);
	return result;
}

gcm_func float v2lensq(v2 value)
{
	float result = (value.x * value.x + value.y * value.y);
	return result;
}

gcm_func float v2len(v2 value)
{
	float result = f32sqrt(value.x * value.x + value.y * value.y);
	return result;
}

gcm_func v2 v2lerp(v2 a, v2 b, float t)
{
	v2 result = {f32lerp(a.x, b.x, t), f32lerp(a.y, b.y, t)};
	return result;
}


gcm_func v3 v3add(v3 a, v3 b) {
	v3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
	return result;
}

gcm_func v3 v3adds(v3 a, float b) {
	v3 result = {a.x + b, a.y + b, a.z + b};
	return result;
}

gcm_func v3 v3sub(v3 a, v3 b) {
	v3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
	return result;
}

gcm_func v3 v3subs(v3 a, float b) {
	v3 result = {a.x - b, a.y - b, a.z - b};
	return result;
}

gcm_func v3 v3mul(v3 a, v3 b)
{
	v3 result = {a.x * b.x, a.y * b.y, a.z * b.z};
	return result;
}

gcm_func v3 v3muls(v3 a, float b) {
	v3 result = {a.x * b, a.y * b, a.z * b};
	return result;
}

gcm_func v3 v3div(v3 a, v3 b) {
	v3 result = {a.x / b.x, a.y / b.y, a.z / b.z};
	return result;
}

gcm_func v3 v3divs(v3 a, float b) {
	v3 result = {a.x / b, a.y / b, a.z / b};
	return result;
}

gcm_func float v3dot(v3 a, v3 b) {
	float result = a.x * b.x + a.y * b.y + a.z * b.z;
	return result;
}

gcm_func v3 v3cross(v3 a, v3 b) {
	v3 result = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
	return result;
}

gcm_func v3 v3mod(v3 a, v3 b) {
	v3 result = {f32mod(a.x, b.x), f32mod(a.y, b.y), f32mod(a.z, b.z)};
	return result;
}

gcm_func v3 v3mods(v3 a, float b) {
	v3 result = {f32mod(a.x, b), f32mod(a.y, b), f32mod(a.z, b)};
	return result;
}

gcm_func v3 v3max(v3 a, v3 b) {
	v3 result = {GCM_MAX(a.x, b.x), GCM_MAX(a.y, b.y), GCM_MAX(a.z, b.z)};
	return result;
}

gcm_func v3 v3min(v3 a, v3 b) {
	v3 result = {GCM_MIN(a.x, b.x), GCM_MIN(a.y, b.y), GCM_MIN(a.z, b.z)};
	return result;
}

gcm_func gcm_bool v3equals(v3 a, v3 b) {
	gcm_bool result = (a.x == b.x && a.y == b.y && a.z == b.z);
	return result;
}

gcm_func v3 v3sign(v3 value) {
	v3 result = {f32sign(value.x), f32sign(value.y), f32sign(value.z)};
	return result;
}

gcm_func v3 v3abs(v3 value) {
	v3 result = {GCM_ABS(value.x), GCM_ABS(value.y), GCM_ABS(value.z)};
	return result;
}

gcm_func v3i v3tov3i(v3 value) {
	v3i result = {(int32_t)value.x, (int32_t)value.y, (int32_t)value.z};
	return result;
}

gcm_func v3 v3fill(float value) {
	v3 result = {value, value, value};
	return result;
}

gcm_func v3 v3floor(v3 value) {
	v3 result = {
		f32floor(value.x),
		f32floor(value.y),
		f32floor(value.z),
	};
	return result;
}

gcm_func v3 v3ceil(v3 value) {
	v3 result = {
		f32ceil(value.x),
		f32ceil(value.y),
		f32ceil(value.z),
	};
	return result;
}

gcm_func v3 v3round(v3 value) {
	v3 result = {
		f32round(value.x),
		f32round(value.y),
		f32round(value.z),
	};
	return result;
}

gcm_func v3 v3negate(v3 value) {
	v3 result = {-value.x, -value.y, -value.z};
	return result;
}

gcm_func v3 v3normalise(v3 value) {
	float length = v3len(value);
	if (length > 0)
	{
		length = 1.0f / length;
	}
	v3 result = v3muls(value, length);
	return result;
}

gcm_func float v3normInPlace(v3 *value) {
	float result = v3len(*value);
	float invLength = result;
	if (invLength > 0)
	{
		invLength = 1.0f / invLength;
	}
	*value = v3muls(*value, invLength);
	return result;
}

gcm_func float v3lensq(v3 value)
{
	float result = (value.x * value.x + value.y * value.y + value.z * value.z);
	return result;
}

gcm_func float v3len(v3 value)
{
	float result = f32sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
	return result;
}

gcm_func v3 v3lerp(v3 a, v3 b, float t)
{
	v3 result = {f32lerp(a.x, b.x, t), f32lerp(a.y, b.y, t), f32lerp(a.z, b.z, t)};
	return result;
}


gcm_func v4 v4add(v4 a, v4 b) {
	v4 result = {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
	return result;
}

gcm_func v4 v4adds(v4 a, float b) {
	v4 result = {a.x + b, a.y + b, a.z + b, a.w + b};
	return result;
}

gcm_func v4 v4sub(v4 a, v4 b) {
	v4 result = {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
	return result;
}

gcm_func v4 v4subs(v4 a, float b) {
	v4 result = {a.x - b, a.y - b, a.z - b, a.w - b};
	return result;
}

gcm_func v4 v4mul(v4 a, v4 b)
{
	v4 result = {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
	return result;
}

gcm_func v4 v4muls(v4 a, float b) {
	v4 result = {a.x * b, a.y * b, a.z * b, a.w * b};
	return result;
}

gcm_func v4 v4div(v4 a, v4 b) {
	v4 result = {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
	return result;
}

gcm_func v4 v4divs(v4 a, float b) {
	v4 result = {a.x / b, a.y / b, a.z / b, a.w / b};
	return result;
}

gcm_func float v4dot(v4 a, v4 b) {
	float result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	return result;
}

gcm_func v4 v4mod(v4 a, v4 b) {
	v4 result = {f32mod(a.x, b.x), f32mod(a.y, b.y), f32mod(a.z, b.z), f32mod(a.w, b.w)};
	return result;
}

gcm_func v4 v4mods(v4 a, float b) {
	v4 result = {f32mod(a.x, b), f32mod(a.y, b), f32mod(a.z, b), f32mod(a.w, b)};
	return result;
}

gcm_func v4 v4max(v4 a, v4 b) {
	v4 result = {GCM_MAX(a.x, b.x), GCM_MAX(a.y, b.y), GCM_MAX(a.z, b.z), GCM_MAX(a.w, b.w)};
	return result;
}

gcm_func v4 v4min(v4 a, v4 b) {
	v4 result = {GCM_MIN(a.x, b.x), GCM_MIN(a.y, b.y), GCM_MIN(a.z, b.z), GCM_MIN(a.w, b.w)};
	return result;
}

gcm_func gcm_bool v4equals(v4 a, v4 b) {
	gcm_bool result = (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
	return result;
}

gcm_func v4 v4sign(v4 value) {
	v4 result = {f32sign(value.x), f32sign(value.y), f32sign(value.z), f32sign(value.w)};
	return result;
}

gcm_func v4 v4abs(v4 value) {
	v4 result = {GCM_ABS(value.x), GCM_ABS(value.y), GCM_ABS(value.z), GCM_ABS(value.w)};
	return result;
}

gcm_func v4i v4tov4i(v4 value) {
	v4i result = {(int32_t)value.x, (int32_t)value.y, (int32_t)value.z, (int32_t)value.w};
	return result;
}

gcm_func v4 v4fill(float value) {
	v4 result = {value, value, value, value};
	return result;
}

gcm_func v4 v4fromv3(v3 xyz, float w) {
	v4 result;
	result.xyz = xyz;
	result.w = w;
	return result;
}


gcm_func v4 v4floor(v4 value) {
	v4 result = {
		f32floor(value.x),
		f32floor(value.y),
		f32floor(value.z),
		f32floor(value.w),
	};
	return result;
}

gcm_func v4 v4ceil(v4 value) {
	v4 result = {
		f32ceil(value.x),
		f32ceil(value.y),
		f32ceil(value.z),
		f32ceil(value.w),
	};
	return result;
}

gcm_func v4 v4round(v4 value) {
	v4 result = {
		f32round(value.x),
		f32round(value.y),
		f32round(value.z),
		f32round(value.w),
	};
	return result;
}

gcm_func v4 v4negate(v4 value) {
	v4 result = {-value.x, -value.y, -value.z, -value.w};
	return result;
}

gcm_func v4 v4normalise(v4 value) {
	float length = v4len(value);
	if (length > 0)
	{
		length = 1.0f / length;
	}
	v4 result = v4muls(value, length);
	return result;
}

gcm_func float v4normInPlace(v4 *value) {
	float result = v4len(*value);
	float invLength = result;
	if (invLength > 0)
	{
		invLength = 1.0f / invLength;
	}
	*value = v4muls(*value, invLength);
	return result;
}

gcm_func float v4lensq(v4 value)
{
	float result = (value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w);
	return result;
}

gcm_func float v4len(v4 value)
{
	float result = f32sqrt(value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w);
	return result;
}

gcm_func v4 v4lerp(v4 a, v4 b, float t)
{
	v4 result = {f32lerp(a.x, b.x, t), f32lerp(a.y, b.y, t), f32lerp(a.z, b.z, t), f32lerp(a.w, b.w, t)};
	return result;
}


gcm_func v2i v2iadd(v2i a, v2i b) {
	v2i result = {a.x + b.x, a.y + b.y};
	return result;
}

gcm_func v2i v2iadds(v2i a, int32_t b) {
	v2i result = {a.x + b, a.y + b};
	return result;
}

gcm_func v2i v2isub(v2i a, v2i b) {
	v2i result = {a.x - b.x, a.y - b.y};
	return result;
}

gcm_func v2i v2isubs(v2i a, int32_t b) {
	v2i result = {a.x - b, a.y - b};
	return result;
}

gcm_func v2i v2imul(v2i a, v2i b)
{
	v2i result = {a.x * b.x, a.y * b.y};
	return result;
}

gcm_func v2i v2imuls(v2i a, int32_t b) {
	v2i result = {a.x * b, a.y * b};
	return result;
}

gcm_func v2i v2idiv(v2i a, v2i b) {
	v2i result = {a.x / b.x, a.y / b.y};
	return result;
}

gcm_func v2i v2idivs(v2i a, int32_t b) {
	v2i result = {a.x / b, a.y / b};
	return result;
}

gcm_func v2i v2imod(v2i a, v2i b) {
	v2i result = {i32mod(a.x, b.x), i32mod(a.y, b.y)};
	return result;
}

gcm_func v2i v2imods(v2i a, int32_t b) {
	v2i result = {i32mod(a.x, b), i32mod(a.y, b)};
	return result;
}

gcm_func v2i v2imax(v2i a, v2i b) {
	v2i result = {GCM_MAX(a.x, b.x), GCM_MAX(a.y, b.y)};
	return result;
}

gcm_func v2i v2imin(v2i a, v2i b) {
	v2i result = {GCM_MIN(a.x, b.x), GCM_MIN(a.y, b.y)};
	return result;
}

gcm_func gcm_bool v2iequals(v2i a, v2i b) {
	gcm_bool result = (a.x == b.x && a.y == b.y);
	return result;
}

gcm_func v2i v2isign(v2i value) {
	v2i result = {i32sign(value.x), i32sign(value.y)};
	return result;
}

gcm_func v2i v2iabs(v2i value) {
	v2i result = {GCM_ABS(value.x), GCM_ABS(value.y)};
	return result;
}

gcm_func v2 v2itov2(v2i value) {
	v2 result = {(float)value.x, (float)value.y};
	return result;
}

gcm_func v2i v2ifill(int32_t value) {
	v2i result = {value, value};
	return result;
}


gcm_func v2i v2idivfloor(v2i a, v2i b) {
	v2i result = {i32divfloor(a.x, b.x), i32divfloor(a.y, b.y)};
	return result;
}

gcm_func v2i v2idivfloors(v2i a, int32_t b) {
	v2i result = {i32divfloor(a.x, b), i32divfloor(a.y, b)};
	return result;
}

gcm_func v2i v2idivceil(v2i a, v2i b) {
	v2i result = {i32divceil(a.x, b.x), i32divceil(a.y, b.y)};
	return result;
}

gcm_func v2i v2idivceils(v2i a, int32_t b) {
	v2i result = {i32divceil(a.x, b), i32divceil(a.y, b)};
	return result;
}

gcm_func v2i v2inegate(v2i value) {
	v2i result = {-value.x, -value.y};
	return result;
}


gcm_func v3i v3iadd(v3i a, v3i b) {
	v3i result = {a.x + b.x, a.y + b.y, a.z + b.z};
	return result;
}

gcm_func v3i v3iadds(v3i a, int32_t b) {
	v3i result = {a.x + b, a.y + b, a.z + b};
	return result;
}

gcm_func v3i v3isub(v3i a, v3i b) {
	v3i result = {a.x - b.x, a.y - b.y, a.z - b.z};
	return result;
}

gcm_func v3i v3isubs(v3i a, int32_t b) {
	v3i result = {a.x - b, a.y - b, a.z - b};
	return result;
}

gcm_func v3i v3imul(v3i a, v3i b)
{
	v3i result = {a.x * b.x, a.y * b.y, a.z * b.z};
	return result;
}

gcm_func v3i v3imuls(v3i a, int32_t b) {
	v3i result = {a.x * b, a.y * b, a.z * b};
	return result;
}

gcm_func v3i v3idiv(v3i a, v3i b) {
	v3i result = {a.x / b.x, a.y / b.y, a.z / b.z};
	return result;
}

gcm_func v3i v3idivs(v3i a, int32_t b) {
	v3i result = {a.x / b, a.y / b, a.z / b};
	return result;
}

gcm_func v3i v3imod(v3i a, v3i b) {
	v3i result = {i32mod(a.x, b.x), i32mod(a.y, b.y), i32mod(a.z, b.z)};
	return result;
}

gcm_func v3i v3imods(v3i a, int32_t b) {
	v3i result = {i32mod(a.x, b), i32mod(a.y, b), i32mod(a.z, b)};
	return result;
}

gcm_func v3i v3imax(v3i a, v3i b) {
	v3i result = {GCM_MAX(a.x, b.x), GCM_MAX(a.y, b.y), GCM_MAX(a.z, b.z)};
	return result;
}

gcm_func v3i v3imin(v3i a, v3i b) {
	v3i result = {GCM_MIN(a.x, b.x), GCM_MIN(a.y, b.y), GCM_MIN(a.z, b.z)};
	return result;
}

gcm_func gcm_bool v3iequals(v3i a, v3i b) {
	gcm_bool result = (a.x == b.x && a.y == b.y && a.z == b.z);
	return result;
}

gcm_func v3i v3isign(v3i value) {
	v3i result = {i32sign(value.x), i32sign(value.y), i32sign(value.z)};
	return result;
}

gcm_func v3i v3iabs(v3i value) {
	v3i result = {GCM_ABS(value.x), GCM_ABS(value.y), GCM_ABS(value.z)};
	return result;
}

gcm_func v3 v3itov3(v3i value) {
	v3 result = {(float)value.x, (float)value.y, (float)value.z};
	return result;
}

gcm_func v3i v3ifill(int32_t value) {
	v3i result = {value, value, value};
	return result;
}


gcm_func v3i v3idivfloor(v3i a, v3i b) {
	v3i result = {i32divfloor(a.x, b.x), i32divfloor(a.y, b.y), i32divfloor(a.z, b.z)};
	return result;
}

gcm_func v3i v3idivfloors(v3i a, int32_t b) {
	v3i result = {i32divfloor(a.x, b), i32divfloor(a.y, b), i32divfloor(a.z, b)};
	return result;
}

gcm_func v3i v3idivceil(v3i a, v3i b) {
	v3i result = {i32divceil(a.x, b.x), i32divceil(a.y, b.y), i32divceil(a.z, b.z)};
	return result;
}

gcm_func v3i v3idivceils(v3i a, int32_t b) {
	v3i result = {i32divceil(a.x, b), i32divceil(a.y, b), i32divceil(a.z, b)};
	return result;
}

gcm_func v3i v3inegate(v3i value) {
	v3i result = {-value.x, -value.y, -value.z};
	return result;
}


gcm_func v4i v4iadd(v4i a, v4i b) {
	v4i result = {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
	return result;
}

gcm_func v4i v4iadds(v4i a, int32_t b) {
	v4i result = {a.x + b, a.y + b, a.z + b, a.w + b};
	return result;
}

gcm_func v4i v4isub(v4i a, v4i b) {
	v4i result = {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
	return result;
}

gcm_func v4i v4isubs(v4i a, int32_t b) {
	v4i result = {a.x - b, a.y - b, a.z - b, a.w - b};
	return result;
}

gcm_func v4i v4imul(v4i a, v4i b)
{
	v4i result = {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
	return result;
}

gcm_func v4i v4imuls(v4i a, int32_t b) {
	v4i result = {a.x * b, a.y * b, a.z * b, a.w * b};
	return result;
}

gcm_func v4i v4idiv(v4i a, v4i b) {
	v4i result = {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
	return result;
}

gcm_func v4i v4idivs(v4i a, int32_t b) {
	v4i result = {a.x / b, a.y / b, a.z / b, a.w / b};
	return result;
}

gcm_func v4i v4imod(v4i a, v4i b) {
	v4i result = {i32mod(a.x, b.x), i32mod(a.y, b.y), i32mod(a.z, b.z), i32mod(a.w, b.w)};
	return result;
}

gcm_func v4i v4imods(v4i a, int32_t b) {
	v4i result = {i32mod(a.x, b), i32mod(a.y, b), i32mod(a.z, b), i32mod(a.w, b)};
	return result;
}

gcm_func v4i v4imax(v4i a, v4i b) {
	v4i result = {GCM_MAX(a.x, b.x), GCM_MAX(a.y, b.y), GCM_MAX(a.z, b.z), GCM_MAX(a.w, b.w)};
	return result;
}

gcm_func v4i v4imin(v4i a, v4i b) {
	v4i result = {GCM_MIN(a.x, b.x), GCM_MIN(a.y, b.y), GCM_MIN(a.z, b.z), GCM_MIN(a.w, b.w)};
	return result;
}

gcm_func gcm_bool v4iequals(v4i a, v4i b) {
	gcm_bool result = (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
	return result;
}

gcm_func v4i v4isign(v4i value) {
	v4i result = {i32sign(value.x), i32sign(value.y), i32sign(value.z), i32sign(value.w)};
	return result;
}

gcm_func v4i v4iabs(v4i value) {
	v4i result = {GCM_ABS(value.x), GCM_ABS(value.y), GCM_ABS(value.z), GCM_ABS(value.w)};
	return result;
}

gcm_func v4 v4itov4(v4i value) {
	v4 result = {(float)value.x, (float)value.y, (float)value.z, (float)value.w};
	return result;
}

gcm_func v4i v4ifill(int32_t value) {
	v4i result = {value, value, value, value};
	return result;
}


gcm_func v4i v4idivfloor(v4i a, v4i b) {
	v4i result = {i32divfloor(a.x, b.x), i32divfloor(a.y, b.y), i32divfloor(a.z, b.z), i32divfloor(a.w, b.w)};
	return result;
}

gcm_func v4i v4idivfloors(v4i a, int32_t b) {
	v4i result = {i32divfloor(a.x, b), i32divfloor(a.y, b), i32divfloor(a.z, b), i32divfloor(a.w, b)};
	return result;
}

gcm_func v4i v4idivceil(v4i a, v4i b) {
	v4i result = {i32divceil(a.x, b.x), i32divceil(a.y, b.y), i32divceil(a.z, b.z), i32divceil(a.w, b.w)};
	return result;
}

gcm_func v4i v4idivceils(v4i a, int32_t b) {
	v4i result = {i32divceil(a.x, b), i32divceil(a.y, b), i32divceil(a.z, b), i32divceil(a.w, b)};
	return result;
}

gcm_func v4i v4inegate(v4i value) {
	v4i result = {-value.x, -value.y, -value.z, -value.w};
	return result;
}


gcm_func int32_t i32wrap(int32_t value, int32_t max)
{
	int32_t result = value;
	int32_t num = i32divfloor(value, max);
	result -= max * num;
	return result;
}

gcm_func int32_t i32divfloor(int32_t a, int32_t b)
{
	int32_t d = a / b;
	int32_t result = d;
	if (d * b != a)
	{
		result = d - ((a < 0) ^ (b < 0));
	}
	
	return result;
}

gcm_func int32_t i32divceil(int32_t a, int32_t b)
{
	gcm_bool positive = (b >= 0) == (a >= 0);
	int32_t result = (a / b) + (a % b != 0 && positive);
	return result;
}

gcm_func int32_t i32mod(int32_t a, int32_t b)
{
	int32_t result = a % b;
	if (result < 0)
	{
		result += b;
	}
	return result;
}

gcm_func int32_t i32sign(int32_t value)
{
	int32_t result = 0;
	if (value > 0)
	{
		result = 1;
	}
	else if (value < 0)
	{
		result = -1;
	}
	return result;
}


gcm_func float f32cos(float value)
{
	float result = cosf(value);
	return result;
}

gcm_func float f32sin(float value)
{
	float result = sinf(value);
	return result;
}

gcm_func float f32exp(float value)
{
	float result = expf(value);
	return result;
}

gcm_func float f32log(float value)
{
	float result = logf(value);
	return result;
}

gcm_func float f32log2(float value)
{
	float result = log2f(value);
	return result;
}

gcm_func float f32sqrt(float value)
{
	float result = sqrtf(value);
	return result;
}

gcm_func float f32tan(float value)
{
	float result = tanf(value);
	return result;
}

gcm_func float f32atan(float value)
{
	float result = atanf(value);
	return result;
}

gcm_func float f32atan2(float y, float x)
{
	float result = atan2f(y, x);
	return result;
}

gcm_func float f32asin(float value)
{
	float result = asinf(value);
	return result;
}

gcm_func float f32acos(float value)
{
	float result = acosf(value);
	return result;
}

gcm_func float f32pow(float a, float b)
{
	float result = powf(a, b);
	return result;
}

gcm_func float f32mod(float a, float b)
{
	float result = fmodf(a, b);
	return result;
}

gcm_func float f32sign(float value)
{
	float result = 1;
	if (value == 0)
	{
		result = 0;
	}
	else if (value < 0)
	{
		result = -1;
	}
	return result;
}

gcm_func float f32round(float value)
{
	float result = roundf(value);
	return result;
}

gcm_func float f32floor(float value)
{
	float result = floorf(value);
	return result;
}

gcm_func float f32ceil(float value)
{
	float result = ceilf(value);
	return result;
}



gcm_func double f64cos(double value)
{
	double result = cos(value);
	return result;
}

gcm_func double f64sin(double value)
{
	double result = sin(value);
	return result;
}

gcm_func double f64exp(double value)
{
	double result = exp(value);
	return result;
}

gcm_func double f64log(double value)
{
	double result = log(value);
	return result;
}

gcm_func double f64log2(double value)
{
	double result = log2(value);
	return result;
}

gcm_func double f64sqrt(double value)
{
	double result = sqrt(value);
	return result;
}

gcm_func double f64tan(double value)
{
	double result = tan(value);
	return result;
}

gcm_func double f64atan(double value)
{
	double result = atan(value);
	return result;
}

gcm_func double f64atan2(double y, double x)
{
	double result = atan2(y, x);
	return result;
}

gcm_func double f64asin(double value)
{
	double result = asin(value);
	return result;
}

gcm_func double f64acos(double value)
{
	double result = acos(value);
	return result;
}

gcm_func double f64pow(double a, double b)
{
	double result = pow(a, b);
	return result;
}

gcm_func double f64mod(double a, double b)
{
	double result = fmod(a, b);
	return result;
}

gcm_func double f64sign(double value)
{
	double result = 1;
	if (value == 0)
	{
		result = 0;
	}
	else if (value < 0)
	{
		result = -1;
	}
	return result;
}

gcm_func double f64round(double value)
{
	double result = round(value);
	return result;
}

gcm_func double f64floor(double value)
{
	double result = floor(value);
	return result;
}

gcm_func double f64ceil(double value)
{
	double result = ceil(value);
	return result;
}



gcm_func float f32lerp(float a, float b, float t)
{
	float result = a + (b - a) * t;
	return result;
}

gcm_func void AnglesToVectors(v3 angles, v3 *forwards, v3 *right, v3 *up)
{
	v3 radAngles = {
		GCM_DEGTORAD(angles.x),
		GCM_DEGTORAD(angles.y),
		GCM_DEGTORAD(angles.z)
	};
	
	v3 cos;
	v3 sin;
	
	cos.x = f32cos(radAngles.x);
	sin.x = f32sin(radAngles.x);
	
	cos.y = f32cos(radAngles.y);
	sin.y = f32sin(radAngles.y);
	
	cos.z = f32cos(radAngles.z);
	sin.z = f32sin(radAngles.z);
	
	if (forwards)
	{
		forwards->x = cos.x * cos.y;
		forwards->y = cos.x * sin.y;
		forwards->z = -sin.x;
	}
	
	if (right)
	{
		right->x = (-1 * sin.z * sin.x * cos.y) + (-1 * cos.z * -sin.y);
		right->y = (-1 * sin.z * sin.x * sin.y) + (-1 * cos.z * cos.y);
		right->z = -1 * sin.z * cos.x;
	}
	
	if (up)
	{
		up->x = (cos.z * sin.x * cos.y + -sin.z * -sin.y);
		up->y = (cos.z * sin.x * sin.y + -sin.z * cos.y);
		up->z = cos.z * cos.x;
	}
}

gcm_func v3 VectorToAngles(v3 vector)
{
	v3 result = {0};
	if (vector.y == 0 && vector.x == 0)
	{
		result.yaw = 0;
		if (vector.z > 0)
		{
			result.pitch = 270;
		}
		else
		{
			result.pitch = 90;
		}
	}
	else
	{
		result.yaw = GCM_RADTODEG(f32atan2(vector.y, vector.x));
		if (result.yaw < 0)
		{
			result.yaw += 360;
		}
		
		float tmp = v2len(vector.xy);
		result.pitch = GCM_RADTODEG(f32atan2(-vector.z, tmp));
		if (result.pitch < 0)
		{
			result.pitch += 360;
		}
	}
	
	return result;
}

gcm_func mat4 mat4transpose(mat4 matrix)
{
	mat4 result;
	for (int32_t col = 0; col < 4; col++)
	{
		for (int32_t row = 0; row < 4; row++)
		{
			result.e[col][row] = matrix.e[row][col];
		}
	}
	return result;
}
gcm_func mat4 mat4diagonal(float diagonal)
{
	mat4 result = {0};
	result.e[0][0] = diagonal;
	result.e[1][1] = diagonal;
	result.e[2][2] = diagonal;
	result.e[3][3] = diagonal;
	return result;
}

// NOTE: yoink https://www.mesa3d.org/
gcm_func mat4 mat4invert(mat4 in)
{
	float inv[16];
	float *m = (float *)&in;
	inv[0] = m[5]  * m[10] * m[15] - 
		m[5]  * m[11] * m[14] - 
		m[9]  * m[6]  * m[15] + 
		m[9]  * m[7]  * m[14] +
		m[13] * m[6]  * m[11] - 
		m[13] * m[7]  * m[10];
	
	inv[4] = -m[4]  * m[10] * m[15] + 
		m[4]  * m[11] * m[14] + 
		m[8]  * m[6]  * m[15] - 
		m[8]  * m[7]  * m[14] - 
		m[12] * m[6]  * m[11] + 
		m[12] * m[7]  * m[10];
	
	inv[8] = m[4]  * m[9] * m[15] - 
		m[4]  * m[11] * m[13] - 
		m[8]  * m[5] * m[15] + 
		m[8]  * m[7] * m[13] + 
		m[12] * m[5] * m[11] - 
		m[12] * m[7] * m[9];
	
	inv[12] = -m[4]  * m[9] * m[14] + 
		m[4]  * m[10] * m[13] +
		m[8]  * m[5] * m[14] - 
		m[8]  * m[6] * m[13] - 
		m[12] * m[5] * m[10] + 
		m[12] * m[6] * m[9];
	
	inv[1] = -m[1]  * m[10] * m[15] + 
		m[1]  * m[11] * m[14] + 
		m[9]  * m[2] * m[15] - 
		m[9]  * m[3] * m[14] - 
		m[13] * m[2] * m[11] + 
		m[13] * m[3] * m[10];
	
	inv[5] = m[0]  * m[10] * m[15] - 
		m[0]  * m[11] * m[14] - 
		m[8]  * m[2] * m[15] + 
		m[8]  * m[3] * m[14] + 
		m[12] * m[2] * m[11] - 
		m[12] * m[3] * m[10];
	
	inv[9] = -m[0]  * m[9] * m[15] + 
		m[0]  * m[11] * m[13] + 
		m[8]  * m[1] * m[15] - 
		m[8]  * m[3] * m[13] - 
		m[12] * m[1] * m[11] + 
		m[12] * m[3] * m[9];
	
	inv[13] = m[0]  * m[9] * m[14] - 
		m[0]  * m[10] * m[13] - 
		m[8]  * m[1] * m[14] + 
		m[8]  * m[2] * m[13] + 
		m[12] * m[1] * m[10] - 
		m[12] * m[2] * m[9];
	
	inv[2] = m[1]  * m[6] * m[15] - 
		m[1]  * m[7] * m[14] - 
		m[5]  * m[2] * m[15] + 
		m[5]  * m[3] * m[14] + 
		m[13] * m[2] * m[7] - 
		m[13] * m[3] * m[6];
	
	inv[6] = -m[0]  * m[6] * m[15] + 
		m[0]  * m[7] * m[14] + 
		m[4]  * m[2] * m[15] - 
		m[4]  * m[3] * m[14] - 
		m[12] * m[2] * m[7] + 
		m[12] * m[3] * m[6];
	
	inv[10] = m[0]  * m[5] * m[15] - 
		m[0]  * m[7] * m[13] - 
		m[4]  * m[1] * m[15] + 
		m[4]  * m[3] * m[13] + 
		m[12] * m[1] * m[7] - 
		m[12] * m[3] * m[5];
	
	inv[14] = -m[0]  * m[5] * m[14] + 
		m[0]  * m[6] * m[13] + 
		m[4]  * m[1] * m[14] - 
		m[4]  * m[2] * m[13] - 
		m[12] * m[1] * m[6] + 
		m[12] * m[2] * m[5];
	
	inv[3] = -m[1] * m[6] * m[11] + 
		m[1] * m[7] * m[10] + 
		m[5] * m[2] * m[11] - 
		m[5] * m[3] * m[10] - 
		m[9] * m[2] * m[7] + 
		m[9] * m[3] * m[6];
	
	inv[7] = m[0] * m[6] * m[11] - 
		m[0] * m[7] * m[10] - 
		m[4] * m[2] * m[11] + 
		m[4] * m[3] * m[10] + 
		m[8] * m[2] * m[7] - 
		m[8] * m[3] * m[6];
	
	inv[11] = -m[0] * m[5] * m[11] + 
		m[0] * m[7] * m[9] + 
		m[4] * m[1] * m[11] - 
		m[4] * m[3] * m[9] - 
		m[8] * m[1] * m[7] + 
		m[8] * m[3] * m[5];
	
	inv[15] = m[0] * m[5] * m[10] - 
		m[0] * m[6] * m[9] - 
		m[4] * m[1] * m[10] + 
		m[4] * m[2] * m[9] + 
		m[8] * m[1] * m[6] - 
		m[8] * m[2] * m[5];
	
	double det = (double)m[0] * (double)inv[0]
		+ (double)m[1] * (double)inv[4]
		+ (double)m[2] * (double)inv[8]
		+ (double)m[3] * (double)inv[12];
	
	mat4 result = {};
	if (det != 0)
	{
		det = 1.0 / det;
		
		for (int32_t i = 0; i < 16; i++)
		{
			result.arr[i] = inv[i] * (float)det;
		}
	}
	return result;
}

gcm_func mat4 mat4rotation(v3 eulerAngles)
{
	mat4 result = mat4diagonal(1);
	
	float cosU = f32cos(eulerAngles.z);
	float cosV = f32cos(eulerAngles.x);
	float cosW = f32cos(eulerAngles.y);
	
	float sinU = f32sin(eulerAngles.z);
	float sinV = f32sin(eulerAngles.x);
	float sinW = f32sin(eulerAngles.y);
	
	result.e[0][0] = cosV * cosW;
	result.e[0][1] = cosV * sinW;
	result.e[0][2] = -sinV;
	
	result.e[1][0] = sinU * sinV * cosW - cosU * sinW;
	result.e[1][1] = cosU * cosW + sinU * sinV * sinW;
	result.e[1][2] = sinU * cosV;
	
	result.e[2][0] = sinU * sinW + cosU * sinV * cosW;
	result.e[2][1] = cosU * sinV * sinW - sinU * cosW;
	result.e[2][2] = cosU * cosV;
	
	return result;
}

gcm_func mat4 mat4translation(v3 translation)
{
	mat4 result = mat4diagonal(1);
	result.e[3][0] = translation.x;
	result.e[3][1] = translation.y;
	result.e[3][2] = translation.z;
	return result;
}

gcm_func mat4 mat4scale(v3 scale)
{
	mat4 result = {0};
	result.e[0][0] = scale.x;
	result.e[1][1] = scale.y;
	result.e[2][2] = scale.z;
	result.e[3][3] = 1;
	return result;
}

gcm_func mat4 mat4scalef32(float scale)
{
	mat4 result = {0};
	result.e[0][0] = scale;
	result.e[1][1] = scale;
	result.e[2][2] = scale;
	result.e[3][3] = 1;
	return result;
}

gcm_func mat4 mat4mul(mat4 left, mat4 right)
{
	mat4 result;
	for (int32_t column = 0; column < 4; column++)
	{
		for (int32_t row = 0; row < 4; row++)
		{
			float sum = 0;
			for (int32_t i = 0; i < 4; ++i)
			{
				sum += left.e[i][row] * right.e[column][i];
			}
			
			result.e[column][row] = sum;
		}
	}
	return result;
}

gcm_func v4 mat4mulv4(mat4 matrix, v4 vec)
{
	v4 result;
	for (int32_t row = 0; row < 4; ++row)
	{
		float sum = 0;
		for (int32_t column = 0; column < 4; ++column)
		{
			sum += matrix.e[column][row] * vec.e[column];
		}
		result.e[row] = sum;
	}
	return result;
}

gcm_func mat3 mat3transpose(mat3 matrix)
{
	mat3 result;
	for (int32_t col = 0; col < 3; col++)
	{
		for (int32_t row = 0; row < 3; row++)
		{
			result.e[col][row] = matrix.e[row][col];
		}
	}
	return result;
}

gcm_func mat3 mat3diagonal(float diagonal)
{
	mat3 result = {0};
	result.e[0][0] = diagonal;
	result.e[1][1] = diagonal;
	result.e[2][2] = diagonal;
	return result;
}

// https://stackoverflow.com/a/18504573
gcm_func mat3 mat3invert(mat3 matrix)
{
	float det = matrix.e[0][0] * (matrix.e[1][1] * matrix.e[2][2] - matrix.e[2][1] * matrix.e[1][2])
		- matrix.e[0][1] * (matrix.e[1][0] * matrix.e[2][2] - matrix.e[1][2] * matrix.e[2][0])
		+ matrix.e[0][2] * (matrix.e[1][0] * matrix.e[2][1] - matrix.e[1][1] * matrix.e[2][0]);
	
	float invdet = 1.0f / det;
	
	mat3 result;
	result.e[0][0] = (matrix.e[1][1] * matrix.e[2][2] - matrix.e[2][1] * matrix.e[1][2]) * invdet;
	result.e[0][1] = (matrix.e[0][2] * matrix.e[2][1] - matrix.e[0][1] * matrix.e[2][2]) * invdet;
	result.e[0][2] = (matrix.e[0][1] * matrix.e[1][2] - matrix.e[0][2] * matrix.e[1][1]) * invdet;
	result.e[1][0] = (matrix.e[1][2] * matrix.e[2][0] - matrix.e[1][0] * matrix.e[2][2]) * invdet;
	result.e[1][1] = (matrix.e[0][0] * matrix.e[2][2] - matrix.e[0][2] * matrix.e[2][0]) * invdet;
	result.e[1][2] = (matrix.e[1][0] * matrix.e[0][2] - matrix.e[0][0] * matrix.e[1][2]) * invdet;
	result.e[2][0] = (matrix.e[1][0] * matrix.e[2][1] - matrix.e[2][0] * matrix.e[1][1]) * invdet;
	result.e[2][1] = (matrix.e[2][0] * matrix.e[0][1] - matrix.e[0][0] * matrix.e[2][1]) * invdet;
	result.e[2][2] = (matrix.e[0][0] * matrix.e[1][1] - matrix.e[1][0] * matrix.e[0][1]) * invdet;
	return result;
}

gcm_func mat3 mat3translation2d(v2 translation)
{
	mat3 result = mat3diagonal(1);
	result.e[2][0] = translation.x;
	result.e[2][1] = translation.y;
	return result;
}

gcm_func mat3 mat3scale(v3 scale)
{
	mat3 result = {0};
	result.e[0][0] = scale.x;
	result.e[1][1] = scale.y;
	result.e[2][2] = scale.z;
	return result;
}

gcm_func mat3 mat3scale2d(v2 scale)
{
	mat3 result = {0};
	result.e[0][0] = scale.x;
	result.e[1][1] = scale.y;
	result.e[2][2] = 1;
	return result;
}

gcm_func mat3 mat3scalef32(float scale)
{
	mat3 result = {0};
	result.e[0][0] = scale;
	result.e[1][1] = scale;
	result.e[2][2] = scale;
	return result;
}

gcm_func mat3 mat3mul(mat3 left, mat3 right)
{
	mat3 result;
	for (int32_t column = 0; column < 3; column++)
	{
		for (int32_t row = 0; row < 3; row++)
		{
			float sum = 0;
			for (int32_t i = 0; i < 3; ++i)
			{
				sum += left.e[i][row] * right.e[column][i];
			}
			
			result.e[column][row] = sum;
		}
	}
	return result;
}

gcm_func v3 mat3mulv3(mat3 matrix, v3 vec)
{
	v3 result;
#if 0
	for (int32_t row = 0; row < 3; row++)
	{
		float sum = 0;
		for (int32_t column = 0; column < 3; column++)
		{
			sum += matrix.e[column][row] * vec.e[column ];
		}
		
		result.e[row] = sum;
	}
#else
	result.x = vec.e[0] * matrix.col[0].x;
    result.y = vec.e[0] * matrix.col[0].y;
    result.z = vec.e[0] * matrix.col[0].z;
	
    result.x += vec.e[1] * matrix.col[1].x;
    result.y += vec.e[1] * matrix.col[1].y;
    result.z += vec.e[1] * matrix.col[1].z;
	
    result.x += vec.e[2] * matrix.col[2].x;
    result.y += vec.e[2] * matrix.col[2].y;
    result.z += vec.e[2] * matrix.col[2].z;
	
#endif
	return result;
}

gcm_func mat3 mat3rotate(float angle, v3 axis)
{
	mat3 result = mat3diagonal(1);
	
	float sinTheta = f32sin(GCM_DEGTORAD(angle));
	float cosTheta = f32cos(GCM_DEGTORAD(angle));
	float cosValue = 1.0f - cosTheta;
	
	result.e[0][0] = (axis.x * axis.x * cosValue) + cosTheta;
	result.e[0][1] = (axis.x * axis.y * cosValue) + (axis.z * sinTheta);
	result.e[0][2] = (axis.x * axis.z * cosValue) - (axis.y * sinTheta);
	
	result.e[1][0] = (axis.y * axis.x * cosValue) - (axis.z * sinTheta);
	result.e[1][1] = (axis.y * axis.y * cosValue) + cosTheta;
	result.e[1][2] = (axis.y * axis.z * cosValue) + (axis.x * sinTheta);
	
	result.e[2][0] = (axis.z * axis.x * cosValue) + (axis.y * sinTheta);
	result.e[2][1] = (axis.z * axis.y * cosValue) - (axis.x * sinTheta);
	result.e[2][2] = (axis.z * axis.z * cosValue) + cosTheta;
	
	return result;
}

gcm_func mat4 mat3tomat4(mat3 matrix)
{
	mat4 result = mat4diagonal(1);
	for (int32_t row = 0; row < 3; row++)
	{
		result.col[row].xyz = matrix.col[row];
	}
	return result;
}

gcm_func uint64_t NextPowerOf2(uint64_t value)
{
	uint64_t result = value;
	if (result)
	{
		result--;
		result |= result>>1;
		result |= result>>2;
		result |= result>>4;
		result |= result>>8;
		result |= result>>16;
		result |= result>>32;
		result++;
	}
	return result;
}
#undef GC_MATHS_IMPLEMENTATION
#endif // GC_MATHS_IMPLEMENTATION
