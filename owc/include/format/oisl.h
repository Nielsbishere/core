#pragma once
#include "template/enum.h"

namespace oi {

	enum class SLVersion : u8 {
		UNDEFINED = 0,
		V1 = 1
	};

	struct SLHeader {

		u32 magicNumber;

		u16 keys;
		SLVersion version;
		u8 useDefaultCharset : 1;
		u8 flags : 7;

		u32 strings;

	};

	struct SLFile {

		using SizeType = u8;

		static constexpr u32 magicNumber = "oiSL"_magicNum;
		static constexpr size_t maxStringSize = SizeType(-1);

		//SLHeader header;
		String keyset;
		//Array<SizeType> stringLengths;
		Array<String> strings;				//Bitset of getBitDepth() per character indexing into keyset

		size_t size;

		SLFile(const String &keyset, const Array<String> &strings);
		SLFile();

		u32 lookup(const String &string) const;
		u32 add(const String &string);

		size_t getBitDepth() const;
		size_t getStringsBitCount() const;
		size_t getStringsDataSize() const;

		static bool read(String path, SLFile &file);
		static bool read(Buffer data, SLFile &file);

		static Buffer write(SLFile &file);	//Creates new buffer
		static bool write(SLFile &file, String path);
	};

}