#pragma once
#include "vulkan/vulkan.h"
#include <vector>

namespace oi {

	namespace gc {

		class RenderTarget;

		struct RenderTargetExt {

			typedef RenderTarget BaseType;

			VkRenderPass renderPass = VK_NULL_HANDLE;
			std::vector<VkFramebuffer> frameBuffer;

		};

	}

}