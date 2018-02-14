#pragma once

#include "Utils/BinaryHelper.h"
#include "OString.h"

namespace oi {

	//!Buffer stores an address and the length of the objects at that address
	//There is no problem with copying it; it doesn't copy the data
	class Buffer {

		friend class CopyBuffer;

	public:

		Buffer();						//Null buffer
		Buffer(u32 length);				//New data
		Buffer(u8 *data, u32 length);	//Copy data
		Buffer(OString str);			//Copy data

		Buffer offset(u32 i) const;

		static Buffer construct(u8 *initData, u32 length);			//Turns the data into a buffer

		void copy(Buffer buf);
		bool copy(Buffer buf, u32 bytesToCopy, u32 bufferOffset, u32 where);

		bool deconstruct();											//Gets rid of data

		operator std::string() const;								//To hex
		std::string toOctal() const;
		std::string toBinary() const;

		Buffer operator+(u32 off) const;
		Buffer subbuffer(u32 offset, u32 length) const;

		template<typename T = u8>
		T &operator[](u32 where) {									//Gets data at location
			return *(T*)&data[where];
		}

		template<typename T = u8>
		T get(u32 where) const {											//Gets data at location (no reference)
			return *(T*)&data[where];
		}

		std::string getString(u32 where, u32 length) const;
		bool setString(u32 where, std::string str);

		u32 size() const;

		static Buffer readFile(OString where);

		u8 *addr() { return data; }

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

		Buffer &toBuffer() const;

	private:

		CopyBuffer &copy(const CopyBuffer &cb);
	};

}