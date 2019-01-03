#pragma once
#include "vulkan/vulkan.h"
#include "types/string.h"
#include "objects/vkgpubuffer.h"
#include "objects/render/vkcommandlist.h"

namespace oi {
	
	namespace gc {
		
		class Graphics;

		struct GraphicsExt {

			typedef Graphics BaseType;

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
			std::vector<std::vector<GPUBufferExt>> stagingBuffers;

			u32 current = 0, frames = 0;
			u32 queueFamilyIndex = u32_MAX;

			PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2 = nullptr;
			PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2 = nullptr;

			#ifdef __RAYTRACING__
				PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV = nullptr;
			#endif

			#ifdef __DEBUG__

				VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;

				#ifdef __WINDOWS__
					PFN_vkSetDebugUtilsObjectNameEXT debugNames = nullptr;
				#endif

			#endif
			
		};

	}

}