#include "types/bitset.h"
#include "utils/log.h"
using namespace oi;

/* BitsetRef */

BitsetRef::BitsetRef(Bitset *bitset, size_t bit) : bitset(bitset), bit(bit) {}
BitsetRef::BitsetRef() : BitsetRef(nullptr, 0) {}

BitsetRef::operator bool() const {
	return bitset->fetch((u32) bit);
}

BitsetRef &BitsetRef::operator=(bool other) {

	u8 &og = bitset->at((u32)bit);
	u8 mask = 1 << (7 - (u32)bit % 8);

	if (other)
		og |= mask;
	else
		og &= ~mask;

	return *this;
}

bool BitsetRef::operator!=(bool other) const {
	return operator bool() != other;
}

bool BitsetRef::operator==(bool other) const {
	return operator bool() == other;
}

bool BitsetRef::operator!=(const BitsetRef &other) const {
	return operator bool() != other.operator bool();
}

bool BitsetRef::operator==(const BitsetRef &other) const {
	return operator bool() == other.operator bool();
}

BitsetRef &BitsetRef::flip() {
	return (*this) = !operator bool();
}

BitsetRef &BitsetRef::operator^=(const BitsetRef &other) {
	return (*this) = operator bool() != other.operator bool();
}

BitsetRef &BitsetRef::operator|=(const BitsetRef &other) {
	return (*this) = operator bool() || other.operator bool();
}

BitsetRef &BitsetRef::operator&=(const BitsetRef &other) {
	return (*this) = operator bool() && other.operator bool();
}

BitsetRef &BitsetRef::operator^=(bool other) {
	return (*this) = operator bool() != other;
}

BitsetRef &BitsetRef::operator|=(bool other) {
	return (*this) = operator bool() || other;
}

BitsetRef &BitsetRef::operator&=(bool other) {
	return (*this) = operator bool() && other;
}

bool BitsetRef::operator^(const BitsetRef &other) const {
	return operator bool() != other.operator bool();
}

bool BitsetRef::operator|(const BitsetRef &other) const {
	return operator bool() || other.operator bool();
}

bool BitsetRef::operator&(const BitsetRef &other) const {
	return operator bool() && other.operator bool();
}

bool BitsetRef::operator||(const BitsetRef &other) const {
	return operator bool() || other.operator bool();
}

bool BitsetRef::operator&&(const BitsetRef &other) const {
	return operator bool() && other.operator bool();
}

bool BitsetRef::operator^(bool other) const {
	return operator bool() != other;
}

bool BitsetRef::operator|(bool other) const {
	return operator bool() || other;
}

bool BitsetRef::operator&(bool other) const {
	return operator bool() && other;
}

bool BitsetRef::operator||(bool other) const {
	return operator bool() || other;
}

bool BitsetRef::operator&&(bool other) const {
	return operator bool() && other;
}

/* Bitset */

Bitset::Bitset() : data(nullptr), bits(0), bytes(0) {}

//Allocate in blocks of 4, so bitwise operators can be executed using uint
Bitset::Bitset(u32 size) : bytes((u32)std::ceil(size / 32.f) * 4), data(nullptr), bits(size) {
	data = new u8[bytes];
	memset(data, 0, bytes);
}

Bitset::Bitset(u32 size, bool def): Bitset(size) {
	memset(data, def ? 0xFF : 0, size);
}

u8 *Bitset::addr() { return data; }
CopyBuffer Bitset::toBuffer() { return CopyBuffer(data, (u32) std::ceil(bits / 8.f)); }

u32 Bitset::getBits() { return bits; }
u32 Bitset::getBytes() { return bytes; }

BitsetRef Bitset::operator[](u32 i) {

	if (i >= bits)
		return {};

	return BitsetRef(this, i);
}

Bitset::~Bitset() {
	if (data != nullptr)
		delete[] data;
}

Bitset::Bitset(const Bitset &other) { copy(other); }
Bitset &Bitset::operator=(const Bitset &other) {
	copy(other);
	return *this;
}

Bitset &Bitset::flip() {
	u32 *uarr = (u32*)data;
	for (u32 i = 0; i < bytes / 4; ++i)
		uarr[i] = ~uarr[i];
	return *this;
}

Bitset &Bitset::operator^=(bool other) {
	u32 *uarr = (u32*)data;
	u32 c = other ? 0xFFFFFFFFU : 0;
	for (u32 i = 0; i < bytes / 4; ++i)
		uarr[i] ^= c;
	return *this;
}

Bitset &Bitset::operator|=(bool other) {
	u32 *uarr = (u32*)data;
	u32 c = other ? 0xFFFFFFFFU : 0;
	for (u32 i = 0; i < bytes / 4; ++i)
		uarr[i] |= c;
	return *this;
}

Bitset &Bitset::operator&=(bool other) {
	u32 *uarr = (u32*)data;
	u32 c = other ? 0xFFFFFFFFU : 0;
	for (u32 i = 0; i < bytes / 4; ++i)
		uarr[i] &= c;
	return *this;
}

Bitset Bitset::operator^(bool other) const {
	Bitset cpy = *this;
	return cpy ^= other;
}

Bitset Bitset::operator|(bool other) const {
	Bitset cpy = *this;
	return cpy |= other;
}

Bitset Bitset::operator&(bool other) const {
	Bitset cpy = *this;
	return cpy &= other;
}

Bitset Bitset::operator^(const Bitset &other) const {
	Bitset cpy = *this;
	return cpy ^= other;
}

Bitset Bitset::operator|(const Bitset &other) const {
	Bitset cpy = *this;
	return cpy |= other;
}

Bitset Bitset::operator&(const Bitset &other) const {
	Bitset cpy = *this;
	return cpy &= other;
}

bool Bitset::operator==(const Bitset &other) const {
	return bits == other.bits && memcmp(data, other.data, bytes) == 0;
}

bool Bitset::operator!=(const Bitset &other) const {
	return !operator==(other);
}

Bitset Bitset::operator~() const {
	Bitset cpy = *this;
	return cpy.flip();
}

Bitset &Bitset::operator^=(const Bitset &other) {

	//TODO: Optimize this

	u32 sbytes = bytes <= other.bytes ? bytes : other.bytes;

	for (u32 i = 0; i < sbytes; ++i)
		data[i] ^= other.data[i];

	for (u32 i = sbytes * 8; i < bits; ++i)
		operator[](i) ^= i >= other.bits ? false : other.fetch(i);

	return *this;

}

Bitset &Bitset::operator|=(const Bitset &other) {

	//TODO: Optimize this

	u32 sbytes = bytes <= other.bytes ? bytes : other.bytes;

	for (u32 i = 0; i < sbytes; ++i)
		data[i] |= other.data[i];

	for (u32 i = sbytes * 8; i < bits; ++i)
		operator[](i) |= i >= other.bits ? false : other.fetch(i);

	return *this;

}

Bitset &Bitset::operator&=(const Bitset &other) {

	//TODO: Optimize this

	u32 sbytes = bytes <= other.bytes ? bytes : other.bytes;

	for (u32 i = 0; i < sbytes; ++i)
		data[i] &= other.data[i];

	for (u32 i = sbytes * 8; i < bits; ++i)
		operator[](i) &= i >= other.bits ? false : other.fetch(i);

	return *this;

}

void Bitset::write(std::vector<u32> &values, u32 bitsPerVal) {

	//TODO: Optimize this

	if (bits != bitsPerVal * values.size())
		Log::throwError<Bitset, 0x0>("Couldn't write values to bitset; bitset didn't have enough space");

	for (u32 i = 0; i < (u32) values.size() * bitsPerVal; ++i) 
		operator[](i) = (values[i / bitsPerVal] & (1 << (bitsPerVal - 1 - i % bitsPerVal))) != 0;

}

void Bitset::read(std::vector<u32> &values, u32 bitsPerVal) {

	if (bits != bitsPerVal * values.size())
		Log::throwError<Bitset, 0x1>("Couldn't read values from bitset; bitset didn't have enough space");

	memset(values.data(), 0, values.size() * sizeof(u32));

	u32 *dest = values.data(), *src = (u32*) data;

	u32 mask = (1 << bitsPerVal) - 1, offset = 0;

	for (u32 i = 0, len = (u32)values.size(); i < len; ++i) {

		u32 leftIndex = offset / 32;
		u32 rightIndex = (offset + bitsPerVal) / 32;
		u32 localOff = offset % 32;

		u32 toEnd = 32 - bitsPerVal;
		u32 leftBits = toEnd - localOff;
		u32 rightBits = localOff - toEnd;

		u32 left = src[leftIndex];
		left = ((left & 0xFF) << 24) | ((left & 0xFF00) << 8) | ((left & 0xFF0000) >> 8) | ((left & 0xFF000000) >> 24);

		if (leftBits <= toEnd)												//Sample from first uint
			*(dest + i) = (left & (mask << leftBits)) >> leftBits;
		else {																//Sample from both uints

			u32 right = src[rightIndex];
			right = ((right & 0xFF) << 24) | ((right & 0xFF00) << 8) | ((right & 0xFF0000) >> 8) | ((right & 0xFF000000) >> 24);

			*(dest + i) = ((left & (mask >> rightBits)) << rightBits) | ((right & (mask << (32 - rightBits))) >> (32 - rightBits));

		}

		offset += bitsPerVal;

	}

}

String Bitset::toString() {
	return Buffer::construct(data, bytes).toHex();
}

u8 &Bitset::at(u32 bit) { return data[bit / 8]; }

void Bitset::copy(const Bitset &other) {
	if (other.data == nullptr) {
		data = nullptr;
		bits = bytes = 0;
	} else {
		data = new u8[bytes = other.bytes];
		bits = other.bits;
		memcpy(data, other.data, bytes);
	}
}