#pragma once

#include <template/enum.h>

namespace oi {

	DEnum(SLHeaderVersion, u8,
		Undefined = 0, v1 = 1
	);

	enum class SLHeaderFlags {
		NONE = 0,
		USE_DEFAULT = 1
	};

	struct SLHeader {

		char header[4];

		u8 version;		//SLHeaderVersion_s
		u8 perChar;
		u8 keys;
		u8 flags;

		u16 names;
		u16 length;

	};

	struct SLString {

		u8 padding;
		u8 size;
		u16 start;

		SLString(u8 size, u16 start) : padding(0), size(size), start(start) {}
		SLString() : SLString(0, 0) {}

	};

	struct SLFile {

		SLHeader header;
		String keyset;
		std::vector<String> names;

		u32 size;

	};

	struct oiSL {

		static bool read(String path, SLFile &file);
		static bool read(Buffer data, SLFile &file);

		static Buffer write(String keyset, std::vector<String> names);	//Creates new buffer
		static bool write(String path, String keyset, std::vector<String> names);
	};

}