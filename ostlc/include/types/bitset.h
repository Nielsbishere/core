#pragma once
#include "buffer.h"

namespace oi {

	class Bitset;

	//A class for referencing to a Bitset's bit
	//Please avoid using the individual bitwise operators
	//Only use them when there's no other option (like using a Bitset-wide bitwise operator)
	class BitsetRef {

	public:

		BitsetRef();
		BitsetRef(Bitset *bitset, size_t bit);

		operator bool() const;
		
		BitsetRef &operator=(bool other);

		bool operator!=(bool other) const;
		bool operator==(bool other) const;

		bool operator!=(const BitsetRef &other) const;
		bool operator==(const BitsetRef &other) const;

		BitsetRef &flip();
		BitsetRef &operator^=(const BitsetRef &other);
		BitsetRef &operator|=(const BitsetRef &other);
		BitsetRef &operator&=(const BitsetRef &other);

		BitsetRef &operator^=(bool other);
		BitsetRef &operator|=(bool other);
		BitsetRef &operator&=(bool other);

		bool operator^(const BitsetRef &other) const;
		bool operator|(const BitsetRef &other) const;
		bool operator&(const BitsetRef &other) const;
		bool operator||(const BitsetRef &other) const;
		bool operator&&(const BitsetRef &other) const;

		bool operator^(bool other) const;
		bool operator|(bool other) const;
		bool operator&(bool other) const;
		bool operator||(bool other) const;
		bool operator&&(bool other) const;

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
		CopyBuffer toBuffer();
		String toString();

		u32 getBits();				//Without padding
		u32 getBytes();				//With padding

		inline bool fetch(u32 i) const {
			return (data[i / 8] & (1 << (7 - i % 8))) != 0;
		}

		BitsetRef operator[](u32 i);

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

	private:

		u8 *data;
		u32 bits, bytes;

		void copy(const Bitset &other);

	};

}