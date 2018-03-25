#include "utils/binaryhelper.h"
#include "types/buffer.h"
#include <math.h>
using namespace oi;

bool BinaryHelper::getBit(const Buffer &where, u32 bits) {
	if (ceil(bits / 8.f) > where.size())
		return false;
	return (where.get<u8>(bits / 8) & (0x80 >> (bits % 8))) != 0;
}

bool BinaryHelper::setBit(Buffer &where, u32 bits, bool value) {
	if (ceil(bits / 8.f) > where.size())
		return false;
	u8 mask = (0x80 >> (bits % 8));
	u8 &loc = where.operator[]<u8>(bits / 8);
	loc = (loc & (~mask)) | (value * mask);
	return true;
}

nibble BinaryHelper::getNibble(u8 where, bool offset) {
	return (where & (0xF0 >> (offset * 4))) >> (4 - offset * 4);
}

octal BinaryHelper::getOctal(const Buffer &where, u32 bits) {

	u8 what = where.get<u8>(bits / 8);
	what = (what & (7 << (bits % 8))) >> (bits % 8);

	if (bits % 8 > 5) {

		u8 front = bits / 8 + 1 > where.size() ? 0 : where.get<u8>(bits / 8 + 1);
		
		u32 dif = 5 - bits % 8;

		what |= (front & ((1 << dif) - 1)) << (3 - dif);
	}
	
	return octal(what);
}

char BinaryHelper::hexChar(nibble what) {
	return what < 10 ? '0' + what : (what < 16 ? 'A' + what - 10 : '?');
}