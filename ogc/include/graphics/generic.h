#pragma once

//These structs are only exposed to low-level code and requires including & linking against the API-dependent implementations
//Vulkan for example would define GraphicsExt as 'VkGraphics', while OpenGL would define it as 'GlGraphics'
//Don't assume implementation, and always check for __API__ (__VULKAN__, __OPENGL__, etc.)
//These extensions don't even have to be specified if they don't have an implementation

namespace oi {

	namespace gc {

		//Structs

		struct GraphicsExt;
		struct TextureExt;
		struct RenderTargetExt;
		struct ShaderStageExt;
		struct CommandListExt;
		struct ShaderExt;
		struct ShaderDataExt;
		struct PipelineStateExt;
		struct GPUBufferExt;
		struct SamplerExt;
		struct PipelineExt;

		//Enums

		class TextureFormatExt;
		class TextureUsageExt;
		class ShaderStageTypeExt;
		class TopologyModeExt;
		class FillModeExt;
		class CullModeExt;
		class WindModeExt;
		class GPUBufferTypeExt;
		class ShaderRegisterTypeExt;
		class SamplerWrappingExt;
		class SamplerMinExt;
		class SamplerMagExt;

	}

}