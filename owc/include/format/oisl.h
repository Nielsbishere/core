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

		char header[4];		//oiSL

		u8 version;			//SLHeaderVersion_s
		u8 perChar;
		u8 keys;
		u8 flags;			//SLHeaderFlags_s

		u16 names;
		u16 length;

	};

	struct SLFile {

		SLHeader header;
		String keyset;
		std::vector<String> names;

		u32 size;

		SLFile(String keyset, std::vector<String> names);
		SLFile();

		u32 lookup(String name);
		u32 add(String name);

	};

	struct oiSL {

		static bool read(String path, SLFile &file);
		static bool read(Buffer data, SLFile &file);

		static Buffer write(SLFile &file);	//Creates new buffer
		static bool write(SLFile &file, String path);
	};

}