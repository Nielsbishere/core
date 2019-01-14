#pragma once
#include "vulkan/vulkan.h"
#include "types/string.h"
#include "objects/vkgpubuffer.h"
#include "objects/render/vkcommandlist.h"

namespace oi {

	struct BlockAllocation;

	namespace gc {

		class Graphics;
		struct TextureExt;
		struct GPUMemoryBlockExt;

		struct GraphicsExt {

			typedef Graphics BaseType;

			static constexpr u32 memoryBlockSize = 128 * 1024 * 1024;

			VkInstance instance = VK_NULL_HANDLE;
			VkPhysicalDevice pdevice = VK_NULL_HANDLE;
			VkDevice device = VK_NULL_HANDLE;
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			VkQueue queue = VK_NULL_HANDLE;
			VkSwapchainKHR swapchain = VK_NULL_HANDLE;
			VkCommandPool pool = VK_NULL_HANDLE;

			VkPhysicalDeviceFeatures pfeatures{};
			VkPhysicalDeviceProperties2 pproperties{};
			VkPhysicalDeviceMemoryProperties pmemory{};

			VkColorSpaceKHR colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
			VkFormat colorFormat = VK_FORMAT_UNDEFINED;

			std::vector<VkFence> presentFence;
			std::vector<VkSemaphore> submitSemaphore, swapchainSemaphore;

			CommandList *stagingCmdList;
			std::vector<std::unordered_map<String, GPUBufferExt>> stagingBuffers;

			u32 current = 0, frames = 0;
			u32 queueFamilyIndex = u32_MAX;

			PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2 = nullptr;
			PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2 = nullptr;

			std::vector<GPUMemoryBlockExt*> memoryBlocks;

			#ifdef __RAYTRACING__
				PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV = nullptr;
			#endif

			#ifdef __DEBUG__

				VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;

				#ifdef __WINDOWS__
					PFN_vkSetDebugUtilsObjectNameEXT debugNames = nullptr;
				#endif

			#endif

			void alloc(GPUBufferExt &ext, GPUBufferType type, String name, bool isStaging = false);
			void alloc(TextureExt &ext, String name);
			
			GPUMemoryBlockExt *alloc(const std::tuple<VkMemoryPropertyFlagBits, VkMemoryRequirements, VkMemoryDedicatedRequirementsKHR> &requirements, String &resourceName, u32 &offset, BlockAllocation &allocation, Buffer &mapped, std::pair<VkImage, VkBuffer> res);
			void dealloc(GPUMemoryBlockExt *block, BlockAllocation allocation);

			void dealloc(GPUBufferExt &ext, String name);
			void dealloc(TextureExt &ext, String name);

		};

	}

}