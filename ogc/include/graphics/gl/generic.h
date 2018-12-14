#pragma once

//The following defines the GL dependent structs and enum structs
//Vk/Gl/Dx/Mt/etc. <className> as <className>Ext
//Like GraphicsExt can be VkGraphics, GlGraphics, DxGraphics, MtGraphics
//Please don't assume that it is either, it should only be used for GL dependent functions
//If the extension isn't supported for the API, it will be set to <yy>Null
//CommandListExt isn't supported for OpenGL; it will just route the functions to GL, which is why it points to GlNull

#ifdef __VULKAN__

#include "vulkan.h"

namespace oi {

	namespace gc {

		struct VkNull {};

		///All class extensions (structs that can go into graphics)

		typedef VkGraphics GraphicsExt;
		typedef VkTexture TextureExt;
		typedef VkRenderTarget RenderTargetExt;
		typedef VkShaderStage ShaderStageExt;
		typedef VkCommandList CommandListExt;
		typedef VkShader ShaderExt;
		typedef VkPipelineState PipelineStateExt;
		typedef VkPipeline PipelineExt;
		typedef VkGBuffer GBufferExt;
		typedef VkSampler SamplerExt;
		
		///All enum struct implementations

		typedef VkTextureFormat TextureFormatExt;
		typedef VkTextureUsage TextureUsageExt;
		typedef VkShaderStageType ShaderStageTypeExt;
		typedef VkTopologyMode TopologyModeExt;
		typedef VkFillMode FillModeExt;
		typedef VkCullMode CullModeExt;
		typedef VkWindMode WindModeExt;
		typedef VkNull BlendModeExt;
		typedef VkNull DepthModeExt;
		typedef VkGBufferType GBufferTypeExt;
		typedef VkShaderRegisterType ShaderRegisterTypeExt;
		typedef VkSamplerWrapping SamplerWrappingExt;
		typedef VkSamplerMin SamplerMinExt;
		typedef VkSamplerMag SamplerMagExt;

	};
}

#elif __OpenGL__

#include "opengl.h"

namespace oi {

	namespace gc {

		struct GlNull {};

		///All class extensions (structs that can go into graphics)

		typedef GlGraphics GraphicsExt;
		typedef GlTexture TextureExt;
		typedef GlRenderTarget RenderTargetExt;
		typedef GlShaderStage ShaderStageExt;
		typedef GlNull CommandListExt;
		typedef GlShader ShaderExt;

		///All DEnum defines

		typedef GlTextureFormat TextureFormatExt;
		typedef GlTextureUsage TextureUsageExt;
		typedef GlShaderStageType ShaderStageTypeExt;

	};
}

#endif