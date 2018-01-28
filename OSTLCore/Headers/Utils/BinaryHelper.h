#pragma once
#include "Types/Generic.h"

namespace oi {

	class Buffer;

	class BinaryHelper {

	private:

		static constexpr uint32_t le_test_one = 1;
		static constexpr uint8_t le_test_one_cast = (const uint8_t&)le_test_one;

	public:

		static bool getBit(const Buffer &where, u32 bits);
		static bool setBit(Buffer &where, u32 bits, bool value);

		static nibble getNibble(u8 where, bool offset);
		static oct_type getOctal(const Buffer &where, u32 bits);
		static char hexChar(nibble what);

		static constexpr bool isLittleEndian = le_test_one_cast == 1;

	};

}