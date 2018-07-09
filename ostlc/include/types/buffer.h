#pragma once

#include "utils/binaryhelper.h"
#include "string.h"

namespace oi {

	//!Buffer stores an address and the length of the objects at that address
	//There is no problem with copying it; it doesn't copy the data
	class Buffer {

		friend class CopyBuffer;

	public:

		Buffer();													//Null buffer
		Buffer(u32 length);											//New data
		Buffer(u8 *data, u32 length);								//Copy data
		Buffer(std::vector<u8> dat);								//Copy data
		Buffer(std::vector<std::vector<u8>> dat);					//Copy data
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
		u32 getBits(u32 bitoff, u32 bits);
		void setBit(u32 bitoff, bool value);
		bool getBit(u32 bitoff);

		Buffer operator+(u32 off) const;
		Buffer subbuffer(u32 offset, u32 length) const;

		//Merge the two buffers into one
		Buffer &operator+=(Buffer other);

		template<typename T = u8>
		T &operator[](u32 where) {									//Gets data at location
			return *(T*)&data[where];
		}

		template<typename T = u8>
		T get(u32 where) const {									//Gets data at location (no reference)
			return *(T*)&data[where];
		}

		String getString(u32 where, u32 length) const;
		bool setString(u32 where, String str);

		u32 size() const;

		u8 *addr();

		std::vector<u8> toArray();

	protected:

		u8 *data;
		u32 length;
	};

	//!CopyBuffer copies around the buffer with the object
	//Useful for arrays of different sizes for example
	class CopyBuffer : public Buffer {

	public:

		CopyBuffer(u32 length);
		CopyBuffer(u8 *initData, u32 length);
		CopyBuffer(Buffer buf);
		~CopyBuffer();

		CopyBuffer(const CopyBuffer &cb);
		CopyBuffer &operator=(const CopyBuffer &cb);
		CopyBuffer(CopyBuffer &&cb);

		Buffer &toBuffer() const;

	private:

		CopyBuffer &copy(const CopyBuffer &cb);
	};

}