#pragma once

#include <format/oisl.h>
#include "oisb.h"

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

		struct SHHeader {

			char header[4];		//oiSH

			u8 version;			//SHHeaderVersion_s
			u8 type;			//SHStageTypeFlag
			u8 shaders;
			u8 p0;

			u8 inputAttributes;
			u8 buffers;
			u8 outputs;
			u8 registers;

			u16 codeSize;
			u16 p1;

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

		enum class SHInputBufferType {
			VERTEX = 0,
			INSTANCE = 1
		};

		struct SHInputVar {

			u16 nameIndex;
			u8 padding = 0;
			u8 type;			//TextureFormat

			SHInputVar(u8 type, u16 nameIndex) : type(type), nameIndex(nameIndex) {}
			SHInputVar() : SHInputVar(0, 0) {}
		};

		enum class SHRegisterType : u8 {
			UNDEFINED = 0, UBO = 1, UBO_WRITE = 2, SSBO = 3, SSBO_WRITE = 4, TEXTURE = 5, IMAGE = 6, SAMPLER = 7
		};

		enum class SHRegisterAccess { 
			COMPUTE = 1, VERTEX = 2, GEOMETRY = 4, FRAGMENT = 8 
		};

		struct SHRegister {

			u8 type;			//SHRegisterType
			u8 access;			//SHRegisterAccess
			u16 representation; //If type is buffer or sampler; represents which buffer to use, same with sampler.

			u16 nameIndex;
			u16 size;			//If the type is an array (array of textures for example)

			SHRegister(u8 type, u8 access, u16 representation, u16 nameIndex, u16 size) : type(type), access(access), representation(representation), nameIndex(nameIndex), size(size) {}
			SHRegister() : SHRegister(0, 0, 0, 0, 0) {}

		};

		struct SHOutput {

			u8 type;			//TextureFormat
			u8 id;
			u16 nameIndex;

			SHOutput(u8 type, u8 id, u16 nameIndex) : type(type), id(id), nameIndex(nameIndex) {}
			SHOutput() : SHOutput(0, 0, 0) {}

		};

		//The contents of an SH file
		struct SHFile {

			SHHeader header;
			std::vector<SHStage> stage;
			std::vector<SHInputVar> ivar;
			std::vector<SHRegister> registers;
			std::vector<SHOutput> outputs;
			SLFile stringlist;
			std::vector<SBFile> buffers;
			std::vector<u8> bytecode;

			u32 size;

			SHFile(std::vector<SHStage> stage, std::vector<SHInputVar> ivar, std::vector<SHRegister> registers, std::vector<SHOutput> outputs, SLFile stringlist, std::vector<SBFile> buffers, std::vector<u8> bytecode) : stage(stage), ivar(ivar), registers(registers), outputs(outputs), stringlist(stringlist), buffers(buffers), bytecode(bytecode) {}
			SHFile() : SHFile({}, {}, {}, {}, {}, {}, {}) {}

		};

		struct oiSH {

			static bool read(String path, SHFile &file);
			static bool read(Buffer buf, SHFile &file);

			static SHFile convert(ShaderInfo info);
			static ShaderInfo convert(Graphics *g, SHFile info);

			static bool write(String path, SHFile &file);
			static Buffer write(SHFile &file);

		};

	}

}