#pragma once
#include "vulkan/vulkan.h"
#include "types/datatype.h"
#include "template/enum.h"
#include "graphics/vkgraphics.h"
#include "memory/blockallocator.h"

namespace oi {

	namespace gc {

		class GPUBuffer;
		class GPUBufferType;
		struct GPUMemoryBlockExt;

		struct GPUBufferAllocationExt {

			GPUMemoryBlockExt *block;
			u32 offset;
			BlockAllocation allocation;

			bool operator==(const GPUBufferAllocationExt &other) const;

		};

		struct GPUBufferExt {

			typedef GPUBuffer BaseType;

			std::vector<VkBuffer> resource;
			std::vector<GPUBufferAllocationExt> allocations;
			u32 alignment = 0;

			static bool isVersioned(GPUBufferType type);
			static bool isStaged(GPUBufferType type);
			static bool isCoherent(GPUBufferType type);

		};

		DEnum(GPUBufferTypeExt, VkBufferUsageFlags,
			UBO = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			SSBO = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			IBO = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VBO = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			CBO = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
		);

	}

}