#pragma once

#include <format/oisl.h>

namespace oi {

	namespace gc {

		class Graphics;
		struct ShaderInfo;

		DEnum(SHHeaderVersion, u8,
			Undefined = 0, v0_0_1 = 1
		);

		enum class SHStageTypeFlag {
			COMPUTE = 0,
			VERTEX = 1,
			FRAGMENT = 2,
			GEOMETRY = 4
		};

		enum class SHInputBufferType {
			VERTEX = 0,
			INSTANCE = 1
		};

		struct SHHeader {

			char header[4];		//oiSH

			u8 version;			//SHHeaderVersion_s
			u8 type;			//SHStageTypeFlag
			u8 shaders;
			u8 inputBuffers;

			u8 inputAttributes;
			u8 buffers;
			u8 outputs;
			u8 registers;

			u16 codeSize;
			u16 padding;

		};

		struct SHStage {

			u8 flags;
			u8 type;			//ShaderStageType
			u16 nameIndex;

			u16 codeIndex;
			u16 codeLength;

			SHStage(u8 flags, u8 type, u16 nameIndex, u16 codeIndex, u16 codeLength) : flags(flags), type(type), nameIndex(nameIndex), codeIndex(codeIndex), codeLength(codeLength) {}
			SHStage() : SHStage(0, 0, 0, 0, 0) {}

		};

		struct SHInputBuffer {

			u8 padding;
			u8 type;			//SHInputBufferType
			u16 size;

			SHInputBuffer(u8 type, u16 size) : type(type), size(size), padding(0) {}
			SHInputBuffer() : SHInputBuffer(0, 0) {}

		};

		struct SHInputVar {

			u16 padding;
			u8 buffer;
			u8 type;			//TextureFormat

			SHInputVar(u8 buffer, u8 type) : type(type), buffer(buffer), padding(0) {}
			SHInputVar() : SHInputVar(0, 0) {}
		};

		//The contents of an SH file
		struct SHFile {

			SHHeader header;
			std::vector<SHStage> stage;
			std::vector<SHInputBuffer> ibuffer;
			std::vector<SHInputVar> ivar;
			SLFile stringlist;
			std::vector<u8> bytecode;

			u32 size;

		};

		struct oiSH {

			static bool read(Graphics *g, String path, ShaderInfo &info);

			static bool read(String path, SHFile &file);
			static bool read(Buffer buf, SHFile &file);

		};

	}

}