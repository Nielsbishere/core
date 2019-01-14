#pragma once

#include "types/string.h"
#include "utils/log.h"
#include "vkgraphics.h"

namespace oi {
	
	namespace gc {

		template<u32 errorId, typename T = GraphicsExt>
		bool vkCheck(VkResult result, String msg) {

			if (result >= VK_SUCCESS) return true;

			switch (result) {

			case VK_ERROR_OUT_OF_HOST_MEMORY:
				Log::error("VkResult: Out of host memory");
				break;

			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				Log::error("VkResult: Out of device memory");
				break;

			case VK_ERROR_INITIALIZATION_FAILED:
				Log::error("VkResult: Initialization failed");
				break;

			case VK_ERROR_DEVICE_LOST:
				Log::error("VkResult: Device lost");
				break;

			case VK_ERROR_MEMORY_MAP_FAILED:
				Log::error("VkResult: Memory map failed");
				break;

			case VK_ERROR_LAYER_NOT_PRESENT:
				Log::error("VkResult: Layer not present");
				break;

			case VK_ERROR_EXTENSION_NOT_PRESENT:
				Log::error("VkResult: Extension not present");
				break;

			case VK_ERROR_FEATURE_NOT_PRESENT:
				Log::error("VkResult: Feature not present");
				break;

			case VK_ERROR_INCOMPATIBLE_DRIVER:
				Log::error("VkResult: Incompatible driver");
				break;

			case VK_ERROR_TOO_MANY_OBJECTS:
				Log::error("VkResult: Too many objects");
				break;

			case VK_ERROR_FORMAT_NOT_SUPPORTED:
				Log::error("VkResult: Format not supported");
				break;

			case VK_ERROR_SURFACE_LOST_KHR:
				Log::error("VkResult: Surface lost");
				break;

			default:
				Log::error("VkResult: Fragmented pool");
				break;

			}

			return Log::throwError<T, errorId>(msg);
		}

		#define vkExtension(x, graphics) PFN_##x x = (PFN_##x) vkGetInstanceProcAddr(graphics->instance, #x); if (x == nullptr) oi::Log::throwError<oi::gc::GraphicsExt, 0x9>("Couldn't get Vulkan extension");
		

		#if defined(__WINDOWS__) && defined(__DEBUG__)

		template<typename T>
		void vkName(GraphicsExt &g, T val, VkObjectType type, String name) {

			const VkDebugUtilsObjectNameInfoEXT namedInfo = {
				VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, // sType
				NULL,                                               // pNext
				type,												// objectType
				(uint64_t)val,										// object
				name.toCString()									// pObjectName
			};

			if(g.debugNames == nullptr)
				return;

			g.debugNames(g.device, &namedInfo);

		}

		#else

		template<typename T>
		void vkName(GraphicsExt&, T, VkObjectType, String) {

		}

		#endif

		//Reserved for allocation
		constexpr VkAllocationCallbacks *vkAllocator = nullptr;

	}

}