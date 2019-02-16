#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdint.h>

using i8 = int8_t;
using u8 = uint8_t;

using octal = uint8_t;
using nibble = uint8_t;

using i16 = int16_t;
using u16 = uint16_t;

using i32 = int32_t;
using u32 = uint32_t;

using i64 = int64_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

constexpr u8 operator ""_u8(unsigned long long test) { return (u8)test; }
constexpr i8 operator ""_i8(unsigned long long test) { return (i8)test; }

constexpr u16 operator ""_u16(unsigned long long test) { return (u16)test; }
constexpr i16 operator ""_i16(unsigned long long test) { return (i16)test; }

constexpr u32 operator ""_u32(unsigned long long test) { return (u32)test; }
constexpr i32 operator ""_i32(unsigned long long test) { return (i32)test; }

constexpr u64 operator ""_u64(unsigned long long test) { return (u64)test; }
constexpr i64 operator ""_i64(unsigned long long test) { return (i64)test; }

constexpr f32 operator ""_f32(long double test) { return (f32)test; }
constexpr f64 operator ""_f64(long double test) { return (f64)test; }

constexpr u8 u8_MAX = 0xFF_u8;
constexpr u8 u8_MIN = 0_u8;
constexpr u16 u16_MAX = 0xFFFF_u16;
constexpr u16 u16_MIN = 0_u16;
constexpr u32 u32_MAX = 0xFFFFFFFF_u32;
constexpr u32 u32_MIN = 0_u32;
constexpr u64 u64_MAX = 0xFFFFFFFFFFFFFFFF_u64;
constexpr u64 u64_MIN = 0_u64;

constexpr i8 i8_MAX = 0x7F_i8;
constexpr i8 i8_MIN = 0x80_i8;
constexpr i16 i16_MAX = 0x7FFF_i16;
constexpr i16 i16_MIN = 0x8000_i16;
constexpr i32 i32_MAX = 0x7FFFFFFF_i32;
constexpr i32 i32_MIN = 0x80000000_i32;
constexpr i64 i64_MAX = 0x7FFFFFFFFFFFFFFF_i64;
constexpr i64 i64_MIN = 0x8000000000000000_i64;

class gbool {

public:

	gbool(u32 val) : val(val) {}
	gbool(bool b = false) : val(b ? 1U : 0U) {}

	operator bool() { return val != 0U; }

private:

	u32 val;

};