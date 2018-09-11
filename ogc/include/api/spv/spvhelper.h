#pragma once
#include "spirv_cross.h"
#include "graphics/texture.h"
#include "graphics/shader.h"

namespace oi {

	namespace gc {

		struct SpvHelper {

			//Get a shader stage type from an extension
			static ShaderStageType pickType(String &extension);

			//Get texture format from type
			static TextureFormat getFormat(spirv_cross::SPIRType type);

			//Insert the member functions of the type into the info buffer
			static void fillStruct(spirv_cross::Compiler &compiler, u32 typeId, ShaderBufferInfo &buffer, ShaderBufferObject *parent);

			//Get shader stage outputs into the ShaderOutputs
			static void getStageOutputs(spirv_cross::Compiler &compiler, spirv_cross::ShaderResources &res, std::vector<ShaderOutput> &output);

			//Get shader stage inputs into the ShaderInputs
			static void getStageInputs(spirv_cross::Compiler &compiler, spirv_cross::ShaderResources &res, std::vector<ShaderVBVar> &output);

			//Convert buffer to ShaderBufferInfo
			static void getBuffer(spirv_cross::Compiler &compiler, spirv_cross::Resource &buffer, ShaderRegister &reg, ShaderBufferInfo &info);

			//Add buffers to oiSH file
			static bool addBuffers(spirv_cross::Compiler &compiler, spirv_cross::ShaderResources &res, ShaderInfo &info, ShaderRegisterAccess access);

		};

	}

}