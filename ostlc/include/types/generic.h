#pragma once

#ifdef NO_LOG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

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

constexpr u8 u8_MAX = 0xFFU;
constexpr u8 u8_MIN = 0U;
constexpr u16 u16_MAX = 0xFFFFU;
constexpr u16 u16_MIN = 0U;
constexpr u32 u32_MAX = 0xFFFFFFFFU;
constexpr u32 u32_MIN = 0U;
constexpr u64 u64_MAX = 0xFFFFFFFFFFFFFFFFU;
constexpr u64 u64_MIN = 0U;

constexpr i8 i8_MAX = 0x7F;
constexpr i8 i8_MIN = (i8)0x80U;
constexpr i16 i16_MAX = 0x7FFF;
constexpr i16 i16_MIN = (i16)0x8000U;
constexpr i32 i32_MAX = 0x7FFFFFFF;
constexpr i32 i32_MIN = (i32)0x80000000U;
constexpr i64 i64_MAX = 0x7FFFFFFFFFFFFFFF;
constexpr i64 i64_MIN = (i64)0x8000000000000000U;

class gbool {

public:

	gbool(u32 val) : val(val) {}
	gbool(bool b = false) : val(b ? 1U : 0U) {}

	operator bool() { return val != 0U; }

private:

	u32 val;

};