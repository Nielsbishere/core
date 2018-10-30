#pragma once

#include "types/buffer.h"
#include "format/oisl.h"
#include "oisb.h"

namespace oi {

	namespace gc {

		class Graphics;
		struct ShaderInfo;

		DEnum(SHHeaderVersion, u8,
			Undefined = 0, v0_1 = 1
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
			u8 padding = 0;

			u8 buffers;
			u8 registers;
			u16 codeSize;

		};

		struct SHStage {

			u8 flags;
			u8 type;			//ShaderStageType
			u16 nameIndex;

			u16 codeIndex;
			u16 codeLength;

			u8 inputs;
			u8 outputs;
			u16 padding = 0;

			SHStage(u8 flags, u8 type, u16 nameIndex, u16 codeIndex, u16 codeLength, u8 inputs, u8 outputs) : flags(flags), type(type), nameIndex(nameIndex), codeIndex(codeIndex), codeLength(codeLength), inputs(inputs), outputs(outputs) {}
			SHStage() : SHStage(0, 0, 0, 0, 0, 0, 0) {}

		};

		struct SHInput {

			u16 nameIndex;
			u8 padding = 0;
			u8 type;			//TextureFormat

			SHInput(u8 type, u16 nameIndex) : type(type), nameIndex(nameIndex) {}
			SHInput() : SHInput(0, 0) {}
		};

		struct SHOutput {

			u8 type;			//TextureFormat
			u8 id;
			u16 nameIndex;

			SHOutput(u8 type, u8 id, u16 nameIndex) : type(type), id(id), nameIndex(nameIndex) {}
			SHOutput() : SHOutput(0, 0, 0) {}

		};

		enum class SHRegisterAccess { 
			COMPUTE = 1, VERTEX = 2, GEOMETRY = 4, FRAGMENT = 8 
		};

		struct SHRegister {

			u8 typeAccess;			//ShaderRegisterType << 4 | SHRegisterAccess
			u8 id;					//Register id
			u16 representation;		//If type is buffer or sampler; represents which buffer to use, same with sampler.

			u16 nameIndex;
			u16 size;				//If the type is an array (array of textures for example)

			SHRegister(u8 type, u8 access, u8 id, u16 representation, u16 nameIndex, u16 size) : typeAccess(((type & 0xF) << 4) | (access & 0xF)), id(id), representation(representation), nameIndex(nameIndex), size(size) {}
			SHRegister() : SHRegister(0, 0, 0, 0, 0, 0) {}

			u8 getType() { return typeAccess >> 4; }
			u8 getAccess() { return typeAccess & 0xF; }

		};

		//The contents of an SH file
		struct SHFile {

			SHHeader header;
			std::vector<SHStage> stage;
			std::unordered_map<u32, std::vector<SHInput>> stageInputs;			//[SHRegisterAccess] = SHInput
			std::unordered_map<u32, std::vector<SHOutput>> stageOutputs;		//[SHRegisterAccess] = SHOutput
			std::vector<SHRegister> registers;
			SLFile stringlist;
			std::vector<SBFile> buffers;
			std::vector<u8> bytecode;

			u32 size;

			SHFile(std::vector<SHStage> stage, std::unordered_map<u32, std::vector<SHInput>> inputs, std::vector<SHRegister> registers, std::unordered_map<u32, std::vector<SHOutput>> outputs, SLFile stringlist, std::vector<SBFile> buffers, std::vector<u8> bytecode) : stage(stage), stageInputs(inputs), registers(registers), stageOutputs(outputs), stringlist(stringlist), buffers(buffers), bytecode(bytecode) {}
			SHFile() : SHFile({}, {}, {}, {}, {}, {}, {}) {}

		};

		enum class ShaderSourceType {
			GLSL,
			HLSL,
			SPV
		};

		class ShaderSource {

			friend struct oiSH;

		private:

			std::unordered_map<String, String> src;
			std::unordered_map<String, CopyBuffer> spv;
			std::vector<String> files;

			ShaderSourceType type;
			String name;

		public:

			//ShaderSource from GLSL/HLSL to oiSH
			//src["vert"] = /* Vertex code */;
			//src["geom"] = /* Geometry code */;
			//src["frag"] = /* Fragment code */;
			//Or
			//src["comp"] = /* Compute code */;
			ShaderSource(String name, ShaderSourceType type, std::unordered_map<String, String> src) : name(name), type(type), src(src) { if (type == ShaderSourceType::SPV) Log::throwError<ShaderSource, 0x0>("Invalid constructor used; SPV isn't text but binary"); }

			//ShaderSource from SPV to oiSH
			//src["vert"] = /* Vertex code */;
			//src["geom"] = /* Geometry code */;
			//src["frag"] = /* Fragment code */;
			//Or
			//src["comp"] = /* Compute code */;
			ShaderSource(String name, std::unordered_map<String, CopyBuffer> spv) : name(name), type(ShaderSourceType::SPV), spv(spv) {}

			//ShaderSource from files to oiSH
			ShaderSource(String name, std::vector<String> files, ShaderSourceType type) : name(name), type(type), files(files) {}

			String getName() { return name; }
			ShaderSourceType getType() { return type; }
			
			const auto &getSources() { return src; }
			const auto &getSpv() { return spv; }
			const auto &getFiles() { return files; }

		};

		struct oiSH {

			static bool read(String path, SHFile &file);
			static bool read(Buffer buf, SHFile &file);

			static SHFile convert(ShaderInfo info);
			static ShaderInfo convert(Graphics *g, SHFile info);

			static SHFile convert(ShaderSource source);
			static ShaderInfo compile(ShaderSource source);

			static bool write(String path, SHFile &file);
			static Buffer write(SHFile &file);

		private:

			static bool compileSource(ShaderSource &source, bool useFile);	//Internal for compiling shader source files (GLSL/HLSL) to SPV

		};

	}

}