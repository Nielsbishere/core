#pragma once

#include "utils/log.h"
#include "utils/binaryhelper.h"

namespace oi {

	class Bitset;

	//!Buffer stores an address and the length of the objects at that address
	//There is no problem with copying it; it doesn't copy the data
	class Buffer {

	public:

		Buffer();													//Null buffer
		Buffer(u32 length);											//New data
		Buffer(u8 *data, u32 length);								//Copy data
		Buffer(String str);											//Copy data
		static Buffer construct(u8 *initData, u32 length);			//Turns the data into a buffer

		Buffer offset(u32 i) const;

		void copy(Buffer buf);
		bool copy(Buffer buf, u32 bytesToCopy, u32 bufferOffset, u32 where);

		bool deconstruct();											//Gets rid of data

		String toHex() const;
		String toOctal() const;
		String toBinary() const;

		void setBits(u32 bitoff, u32 bits, u32 value);
		u32 getBits(u32 bitoff, u32 bits) const;
		void setBit(u32 bitoff, bool value);
		bool getBit(u32 bitoff) const;

		Buffer operator+(u32 off) const;
		Buffer subbuffer(u32 offset, u32 length) const;

		//Uncompress with zlib
		bool uncompress(Buffer result) const;

		//Compress with zlib
		Buffer compress() const;

		//Compress with zlib into buffer
		bool compress(Buffer result) const;

		//Merge the two buffers into one
		Buffer &operator+=(Buffer other);

		//Offset this buffer
		Buffer &operator+=(u32 delta);

		template<typename T = u8>
		T &operator[](u32 where) {									//Gets data at location
			return *(T*)&data[where];
		}

		template<typename T = u8>
		const T &operator[](u32 where) const {
			return *(const T*)&data[where];
		}

		template<typename T = u8>
		T get(u32 where) const {									//Gets data at location (no reference)
			return *(T*)&data[where];
		}

		//Read a type (C-struct only) from this buffer
		//It offsets the buffer to the next element
		//Be sure to keep the base address if you need to delete this buffer
		template<typename T>
		bool read(T &t);

		//Read a type (C-struct only) from this buffer
		//It offsets the buffer to the next element
		//Be sure to keep the base address if you need to delete this buffer
		//T[] data
		template<typename T>
		bool read(std::vector<T> &t, u32 size);

		//Read a type (C-struct only) from this buffer
		//It offsets the buffer to the next element
		//Be sure to keep the base address if you need to delete this buffer
		//u32 size
		//T[] data
		template<typename T>
		bool read(std::vector<T> &t);

		bool read(Bitset &bitset, u32 bits);

		String getString(u32 where, u32 length) const;
		bool setString(u32 where, String str);

		u32 size() const;
		void clear();

		template<typename Cast = u8>
		Cast *addr() const { return (Cast*) data; }

		template<typename Cast = u8>
		Cast *addrEnd() const { return (Cast*)(data + length); }

	protected:

		u8 *data;
		u32 length;
	};

	typedef Array<u8> CopyBuffer;

	template<typename T>
	bool Buffer::read(T &t) {

		if (length < (u32) sizeof(T))
			return Log::error("Couldn't read a type from a buffer; not enough space");

		t = *(T*)data;
		*this = offset((u32) sizeof(T));
		return true;
	}

	//Read a type (C-struct only) from this buffer
	//It offsets the buffer to the next element
	//Be sure to keep the base address if you need to delete this buffer
	//T[] data
	template<typename T>
	bool Buffer::read(std::vector<T> &t, u32 size) {

		if (length < (u32) sizeof(T) * size)
			return Log::error("Couldn't read a vector from a buffer; not enough space");

		t.resize(size);
		memcpy(t.data(), data, sizeof(T) * size);
		*this = offset((u32) sizeof(T) * size);
		return true;
	}

	//Read a type (C-struct only) from this buffer
	//It offsets the buffer to the next element
	//Be sure to keep the base address if you need to delete this buffer
	//u32 size
	//T[] data
	template<typename T>
	bool Buffer::read(std::vector<T> &t) {

		u32 size;

		if (!read(size))
			return Log::error("Couldn't read vector from a buffer; missing size");

		if (!read(t, size))
			return Log::error("Couldn't read vector from a buffer; missing data");

		return true;

	}

}