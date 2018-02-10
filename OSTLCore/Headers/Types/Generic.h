#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

typedef signed __int8 i8;
typedef unsigned __int8 u8;

typedef signed __int16 i16;
typedef unsigned __int16 u16;

typedef signed __int32 i32;
typedef unsigned __int32 u32;

typedef signed __int64 i64;
typedef unsigned __int64 u64;

typedef float f32;
typedef double f64;

typedef u8 nibble;
typedef u8 oct_type;

constexpr u8 u8_MAX = (u8)0xFF;
constexpr u8 u8_MIN = (u8)0;
constexpr u16 u16_MAX = (u16)0xFFFF;
constexpr u16 u16_MIN = (u16)0;
constexpr u32 u32_MAX = (u32)0xFFFFFFFF;
constexpr u32 u32_MIN = (u32)0;
constexpr u64 u64_MAX = (u64)0xFFFFFFFFFFFFFFFF;
constexpr u64 u64_MIN = (u64)0;

constexpr i8 i8_MAX = (u8)0x7F;
constexpr i8 i8_MIN = (u8)0x80;
constexpr i16 i16_MAX = (u16)0x7FFF;
constexpr i16 i16_MIN = (u16)0x8000;
constexpr i32 i32_MAX = (u32)0x7FFFFFFF;
constexpr i32 i32_MIN = (u32)0x80000000;
constexpr i64 i64_MAX = (u64)0x7FFFFFFFFFFFFFFF;
constexpr i64 i64_MIN = (u64)0x8000000000000000;

class gbool {

public:

	gbool(u32 _val) : val(_val) {}
	gbool(bool b = false) : val(b ? 1 : 0) {}

	operator bool() { return val != 0; }

private:

	u32 val;

};

typedef f32 vec4f32[4];
typedef f64 vec4f64[4];
typedef f32 vec3f32[3];
typedef f64 vec3f64[3];
typedef f32 vec2f32[2];
typedef f64 vec2f64[2];

typedef u32 vec4u32[4];
typedef u64 vec4u64[4];
typedef u32 vec3u32[3];
typedef u64 vec3u64[3];
typedef u32 vec2u32[2];
typedef u64 vec2u64[2];
typedef u16 vec4u16[4];
typedef u8 vec4u8[4];
typedef u16 vec3u16[3];
typedef u8 vec3u8[3];
typedef u16 vec2u16[2];
typedef u8 vec2u8[2];

typedef i32 vec4i32[4];
typedef i64 vec4i64[4];
typedef i32 vec3i32[3];
typedef i64 vec3i64[3];
typedef i32 vec2i32[2];
typedef i64 vec2i64[2];
typedef i16 vec4i16[4];
typedef i8 vec4i8[4];
typedef i16 vec3i16[3];
typedef i8 vec3i8[3];
typedef i16 vec2i16[2];
typedef i8 vec2i8[2];