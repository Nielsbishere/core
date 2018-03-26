#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdint.h>

typedef int8_t i8;
typedef uint8_t u8;

typedef int16_t i16;
typedef uint16_t u16;

typedef int32_t i32;
typedef uint32_t u32;

typedef int64_t i64;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef u8 nibble;
typedef u8 octal;

constexpr u8 u8_MAX = (u8)0xFFU;
constexpr u8 u8_MIN = (u8)0U;
constexpr u16 u16_MAX = (u16)0xFFFFU;
constexpr u16 u16_MIN = (u16)0U;
constexpr u32 u32_MAX = (u32)0xFFFFFFFFU;
constexpr u32 u32_MIN = (u32)0U;
constexpr u64 u64_MAX = (u64)0xFFFFFFFFFFFFFFFFU;
constexpr u64 u64_MIN = (u64)0U;

constexpr i8 i8_MAX = (i8)0x7F;
constexpr i8 i8_MIN = (i8)0x80;
constexpr i16 i16_MAX = (i16)0x7FFF;
constexpr i16 i16_MIN = (i16)0x8000;
constexpr i32 i32_MAX = (i32)0x7FFFFFFF;
constexpr i32 i32_MIN = (i32)0x80000000;
constexpr i64 i64_MAX = (i64)0x7FFFFFFFFFFFFFFF;
constexpr i64 i64_MIN = (i64)0x8000000000000000;

///Floating point; if define "USE_DOUBLES" is defined, it will use a 64-bit float, otherwise it uses a 32-bit float.
#ifdef USE_DOUBLES
typedef f64 flp;		//64-bit float; clear define USE_DOUBLES to change it to 32-bit
#else
typedef f32 flp;		//32-bit float; set define USE_DOUBLES to change it to 64-bit
#endif

class gbool {

public:

	gbool(u32 val) : val(val) {}
	gbool(bool b = false) : val(b ? 1U : 0U) {}

	operator bool() { return val != 0U; }

private:

	u32 val;

};