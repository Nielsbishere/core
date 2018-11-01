#pragma once
#include <cstring>
#include <cmath>
#include "buffer.h"

namespace oi {

	//Dynamic bitset

	class Bitset;

	//A class for referencing to a Bitset's bit
	//Please avoid using the individual bitwise operators
	//Only use them when there's no other option (like using a Bitset-wide bitwise operator)
	class BitsetRef {

	public:

		BitsetRef();
		BitsetRef(Bitset *bitset, size_t bit);
		BitsetRef(const Bitset *bitset, size_t bit);

		operator bool() const;
		
		BitsetRef &operator=(bool other);

		bool operator!=(bool other) const { return operator bool() != other; }
		bool operator==(bool other) const { return operator bool() == other; }

		BitsetRef &operator^=(bool other) { return (*this) = operator bool() != other; }
		BitsetRef &operator|=(bool other) { return (*this) = operator bool() || other; }
		BitsetRef &operator&=(bool other) { return (*this) = operator bool() && other; }

		bool operator^(bool other) const { return operator bool() != other; }
		bool operator|(bool other) const { return operator bool() || other; }
		bool operator&(bool other) const { return operator bool() && other; }
		bool operator||(bool other) const { return operator bool() || other; }
		bool operator&&(bool other) const { return operator bool() && other; }

	private:

		Bitset *bitset;
		size_t bit;

	};

	//A class for handling large numbers of flags
	class Bitset {

	public:

		Bitset();
		Bitset(u32 size);
		Bitset(u32 size, bool def);

		u8 *addr();
		CopyBuffer toBuffer() const;
		String toString() const;

		u32 getBits() const;				//Without padding
		u32 getBytes() const;				//With padding

		inline bool fetch(u32 i) const {
			return (data[i / 8] & (1 << (7 - i % 8))) != 0;
		}

		BitsetRef operator[](u32 i);
		bool operator[](u32 i) const;

		~Bitset();
		Bitset(const Bitset &other);
		Bitset &operator=(const Bitset &other);

		Bitset &flip();

		Bitset &operator^=(bool other);
		Bitset &operator|=(bool other);
		Bitset &operator&=(bool other);

		Bitset &operator^=(const Bitset &other);
		Bitset &operator|=(const Bitset &other);
		Bitset &operator&=(const Bitset &other);

		Bitset operator^(bool other) const;
		Bitset operator|(bool other) const;
		Bitset operator&(bool other) const;

		Bitset operator^(const Bitset &other) const;
		Bitset operator|(const Bitset &other) const;
		Bitset operator&(const Bitset &other) const;

		bool operator==(const Bitset &other) const;
		bool operator!=(const Bitset &other) const;

		Bitset operator~() const;

		void write(std::vector<u32> &values, u32 bitsPerVal);
		void read(std::vector<u32> &values, u32 bitsPerVal);

		u8 &at(u32 bit);

		void clear();

	private:

		u8 *data;
		u32 bits, bytes;

		void copy(const Bitset &other);

	};

	//Static Bitset

	template<u32 n>
	class StaticBitset;

	//A class for referencing to a Bitset's bit
	//Please avoid using the individual bitwise operators
	//Only use them when there's no other option (like using a Bitset-wide bitwise operator)
	template<u32 n>
	class StaticBitsetRef {

	public:

		StaticBitsetRef(StaticBitset<n> *bitset, size_t bit) : bitset(bitset), bit(bit) {}
		StaticBitsetRef() {}
		StaticBitsetRef(const StaticBitset<n> *bitset, size_t bit) : bitset((StaticBitset<n>*)bitset), bit(bit) {}

		operator bool() const;

		StaticBitsetRef &operator=(bool other);

		bool operator!=(bool other) const { return operator bool() != other; }
		bool operator==(bool other) const { return operator bool() == other; }

		StaticBitsetRef &operator^=(bool other) { return (*this) = operator bool() != other; }
		StaticBitsetRef &operator|=(bool other) { return (*this) = operator bool() || other; }
		StaticBitsetRef &operator&=(bool other) { return (*this) = operator bool() && other; }

		bool operator^(bool other) const { return operator bool() != other; }
		bool operator|(bool other) const { return operator bool() || other; }
		bool operator&(bool other) const { return operator bool() && other; }
		bool operator||(bool other) const { return operator bool() || other; }
		bool operator&&(bool other) const { return operator bool() && other; }

	private:

		StaticBitset<n> *bitset = nullptr;
		size_t bit = 0;

	};

	//A class for handling large numbers of flags
	template<u32 n>
	class StaticBitset {

	public:

		static constexpr u32 bits = n;
		static constexpr u32 bytes = n % 8 != 0 ? n / 8 + 1 : n / 8;

		StaticBitset() { memset(data, 0, bytes); }
		StaticBitset(bool def) { memset(data, def ? 0xFF : 0x00, bytes); }

		typedef u8 (&ArrayType)[bytes];

		ArrayType getArray() { return data; }
		u8 *addr() { return data; }
		CopyBuffer toBuffer() const { return CopyBuffer((u8*)data, bytes); }
		String toString() const { return Buffer::construct((u8*)data, bytes).toHex(); }

		inline bool fetch(u32 i) const {
			return (data[i / 8] & (1 << (7 - i % 8))) != 0;
		}

		StaticBitsetRef<n> operator[](u32 i) {
			return StaticBitsetRef<n>(this, i);
		}

		bool operator[](u32 i) const {
			return StaticBitsetRef<n>(this, i);
		}

		StaticBitset(const StaticBitset &other) { copy(other); }
		StaticBitset &operator=(const StaticBitset &other) { copy(other); return *this; }

		StaticBitset &flip() {

			u32 *uarr = (u32*)data;

			for (u32 i = 0; i < bytes / 4; ++i)
				uarr[i] = ~uarr[i];

			for (u32 i = 0; i < bytes % 4; ++i)
				data[bytes / 4 * 4 + i] = ~data[bytes / 4 * 4 + i];

			return *this;
		}

		StaticBitset &operator^=(bool other) {

			u32 *uarr = (u32*)data;

			u32 c = other ? 0xFFFFFFFFU : 0;
			u8 c0 = other ? 0xFFU : 0;

			for (u32 i = 0; i < bytes / 4; ++i)
				uarr[i] ^= c;

			for (u32 i = 0; i < bytes % 4; ++i)
				data[bytes / 4 * 4 + i] ^= c0;

			return *this;
		}

		StaticBitset &operator|=(bool other) {

			u32 *uarr = (u32*)data;

			u32 c = other ? 0xFFFFFFFFU : 0;
			u8 c0 = other ? 0xFFU : 0;

			for (u32 i = 0; i < bytes / 4; ++i)
				uarr[i] |= c;

			for (u32 i = 0; i < bytes % 4; ++i)
				data[bytes / 4 * 4 + i] |= c0;

			return *this;
		}

		StaticBitset &operator&=(bool other) {

			u32 *uarr = (u32*)data;

			u32 c = other ? 0xFFFFFFFFU : 0;
			u8 c0 = other ? 0xFFU : 0;

			for (u32 i = 0; i < bytes / 4; ++i)
				uarr[i] &= c;

			for (u32 i = 0; i < bytes % 4; ++i)
				data[bytes / 4 * 4 + i] &= c0;

			return *this;
		}

		StaticBitset &operator^=(const StaticBitset &other) {

			u32 *uarr = (u32*)data;
			const u32 *uarr0 = (const u32*)other.data;

			for (u32 i = 0; i < bytes / 4; ++i)
				uarr[i] ^= uarr0[i];

			for (u32 i = 0; i < bytes % 4; ++i)
				data[bytes / 4 * 4 + i] ^= other.data[bytes / 4 * 4 + i];

			return *this;

		}

		StaticBitset &operator|=(const StaticBitset &other) {

			u32 *uarr = (u32*)data;
			const u32 *uarr0 = (const u32*)other.data;

			for (u32 i = 0; i < bytes / 4; ++i)
				uarr[i] |= uarr0[i];

			for (u32 i = 0; i < bytes % 4; ++i)
				data[bytes / 4 * 4 + i] |= other.data[bytes / 4 * 4 + i];

			return *this;

		}

		StaticBitset &operator&=(const StaticBitset &other) {

			u32 *uarr = (u32*)data;
			const u32 *uarr0 = (const u32*)other.data;

			for (u32 i = 0; i < bytes / 4; ++i)
				uarr[i] &= uarr0[i];

			for (u32 i = 0; i < bytes % 4; ++i)
				data[bytes / 4 * 4 + i] &= other.data[bytes / 4 * 4 + i];

			return *this;

		}

		StaticBitset operator^(bool other) const { StaticBitset cpy = *this; return cpy ^= other; }
		StaticBitset operator|(bool other) const { StaticBitset cpy = *this; return cpy |= other; }
		StaticBitset operator&(bool other) const { StaticBitset cpy = *this; return cpy &= other; }

		StaticBitset operator^(const StaticBitset &other) const { StaticBitset cpy = *this; return cpy ^= other; }
		StaticBitset operator|(const StaticBitset &other) const { StaticBitset cpy = *this; return cpy |= other; }
		StaticBitset operator&(const StaticBitset &other) const { StaticBitset cpy = *this; return cpy &= other; }

		bool operator==(const StaticBitset &other) const { return memcmp(data, other.data, bytes) == 0; }
		bool operator!=(const StaticBitset &other) const { return memcmp(data, other.data, bytes) != 0; }

		u8 &at(u32 bit) { return data[bit / 8]; }

		StaticBitset operator~() const {
			StaticBitset cpy = *this;
			cpy.flip();
			return cpy;
		}

		void clear() {
			memset(data, 0, bytes);
		}

	private:

		u8 data[bytes];

		void copy(const StaticBitset &other) { memcpy(data, other.data, bytes); }

	};

	//StaticBitsetRef definition

	template<u32 n>
	StaticBitsetRef<n>::operator bool() const {
		return bitset->fetch((u32)bit);
	}

	template<u32 n>
	StaticBitsetRef<n> &StaticBitsetRef<n>::operator=(bool other) {

		u8 &og = bitset->at((u32)bit);
		u8 mask = 1 << (7 - (u32)bit % 8);

		if (other)
			og |= mask;
		else
			og &= ~mask;

		return *this;
	}

}