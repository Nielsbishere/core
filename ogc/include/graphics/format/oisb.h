#pragma once

#include <template/enum.h>

namespace oi {

	struct SLFile;

	namespace gc {

		class Graphics;
		struct ShaderBufferInfo;

		DEnum(SBHeaderVersion, u8,
			Undefined = 0, v0_1 = 1
		);

		enum class SBHeaderFlag : u8 {
			IS_WRITE = 0x1U,
			IS_STORAGE = 0x2U,
			IS_ALLOCATED = 0x4U
		};

		struct SBHeader {

			char header[4];

			u8 version;		//SBHeaderVersion_s
			u8 flags;		//SBHeaderFlag; & 0x3 = Buffer type
			u16 padding = 0;

			u16 structs;
			u16 vars;

			u32 bufferSize;

		};

		struct SBStruct {

			u16 nameIndex;
			u16 parent;

			u32 offset;

			u32 arraySize;

			u32 length;

			SBStruct(u16 nameIndex, u16 parent, u32 offset, u32 arraySize, u32 length);
			SBStruct();

		};

		struct SBVar {

			u16 nameIndex;
			u16 parent;

			u32 offset;

			u32 arraySize;

			u8 type;		//TextureFormat
			u8 padding[3];

			SBVar(u16 nameIndex, u16 parent, u32 offset, u32 arraySize, u8 type);
			SBVar();

		};

		struct SBFile {

			SBHeader header;
			std::vector<SBStruct> structs;
			std::vector<SBVar> vars;

			u32 size;

			SBFile(std::vector<SBStruct> structs, std::vector<SBVar> vars);
			SBFile();

		};

		struct oiSB {

			static bool read(String path, SBFile &file);
			static bool read(Buffer data, SBFile &file);
			
			static SBFile convert(ShaderBufferInfo info, SLFile *names = nullptr);
			static ShaderBufferInfo convert(Graphics *g, SBFile file, SLFile *names = nullptr);

			static Buffer write(SBFile file);					//Creates new buffer
			static bool write(String path, SBFile file);
		};

	}
}