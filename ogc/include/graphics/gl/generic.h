#pragma once

//The following defines the GL dependent structs and enum structs
//Vk/Gl/Dx/Mt/etc. <className> as <className>Ext
//Like GraphicsExt can be VkGraphics, GlGraphics, DxGraphics, MtGraphics
//Please don't assume that it is either, it should only be used for GL dependent functions

#ifdef __VULKAN__

#include "vulkan.h"

namespace oi {

	namespace gc {

		///All class extensions (structs that can go into graphics)

		typedef VkGraphics GraphicsExt;
		typedef VkTexture TextureExt;
		typedef VkRenderTarget RenderTargetExt;
		typedef VkShaderStage ShaderStageExt;
		typedef VkCommandList CommandListExt;
		
		///All enum struct implementations

		typedef VkTextureFormat TextureFormatExt;
		typedef VkTextureUsage TextureUsageExt;
		typedef VkShaderStageType ShaderStageTypeExt;

	};
}

#elif __OpenGL__

#include "opengl.h"

namespace oi {

	namespace gc {

		///All class extensions (structs that can go into graphics)

		typedef GlGraphics GraphicsExt;
		typedef GlTexture TextureExt;
		typedef GlRenderTarget RenderTargetExt;
		typedef GlShaderStage ShaderStageExt;

		///All DEnum defines

		typedef GlTextureFormat TextureFormatExt;
		typedef GlTextureUsage TextureUsageExt;
		typedef GlShaderStageType ShaderStageTypeExt;

	};
}

#endif