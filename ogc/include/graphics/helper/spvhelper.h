#pragma once
#include "graphics/objects/texture/texture.h"
#include "graphics/objects/shader/shader.h"

namespace spirv_cross {
	struct SPIRType;
	struct ShaderResources;
	class Compiler;
	struct Resource;
}

namespace oi {

	namespace gc {

		struct SpvHelper {

			//Get a shader stage type from an extension
			static ShaderStageType pickType(const String &extension);

			//Get texture format from type
			static TextureFormat getFormat(spirv_cross::SPIRType type);

			//Insert the member functions of the type into the info buffer
			static void fillStruct(spirv_cross::Compiler &compiler, u32 typeId, ShaderBufferInfo &buffer, u32 parent);

			//Get shader stage outputs into the ShaderOutputs
			static void getStageOutputs(spirv_cross::Compiler &compiler, spirv_cross::ShaderResources &res, std::vector<ShaderOutput> &output);

			//Get shader stage inputs into the ShaderInputs
			static void getStageInputs(spirv_cross::Compiler &compiler, spirv_cross::ShaderResources &res, std::vector<ShaderInput> &output);

			//Convert buffer to ShaderBufferInfo
			static void getBuffer(spirv_cross::Compiler &compiler, spirv_cross::Resource &buffer, ShaderRegister &reg, ShaderBufferInfo &info, String name, bool allocatable);

			//Add buffers to shader
			static bool addBuffers(spirv_cross::Compiler &compiler, spirv_cross::ShaderResources &res, ShaderInfo &info, ShaderRegisterAccess access);

			//Add textures to shader
			static bool addTextures(spirv_cross::Compiler &compiler, spirv_cross::ShaderResources &res, ShaderInfo &info, ShaderRegisterAccess access);

			//Add samples to shader
			static bool addSamplers(spirv_cross::Compiler &compiler, spirv_cross::ShaderResources &res, ShaderInfo &info, ShaderRegisterAccess access);

			//Add resources to shader
			static bool addResources(spirv_cross::Compiler &compiler, ShaderStageType type, ShaderInfo &info, std::vector<ShaderInput> &input, std::vector<ShaderOutput> &output);

			//Add stage to shader
			static bool addStage(CopyBuffer buf, ShaderStageType type, ShaderInfo &info, bool stripDebug);

		};

	}

}