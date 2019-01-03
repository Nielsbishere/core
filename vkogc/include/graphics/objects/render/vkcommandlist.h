#pragma once
#include "vulkan/vulkan.h"
#include <vector>

namespace oi {

	namespace gc {

		struct GraphicsExt;
		class CommandList;

		struct CommandListExt {

			typedef CommandList BaseType;

			VkCommandPool pool = VK_NULL_HANDLE;
			std::vector<VkCommandBuffer> cmds;

			VkCommandBuffer &cmd(GraphicsExt &g);

		};

	}

}