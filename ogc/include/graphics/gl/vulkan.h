#include <vulkan/vulkan.h>
#include <vector>
#include <types/string.h>
#include <utils/log.h>
#include <template/enum.h>

namespace oi {
	
	namespace gc {
		
		class Texture;
		class Graphics;

		struct VkGraphics {
			VkInstance instance;
			VkPhysicalDevice pdevice;
			VkPhysicalDeviceMemoryProperties pmemory;
			VkDevice device;
			VkSurfaceKHR surface;
			VkFormat colorFormat;
			VkColorSpaceKHR colorSpace;
			VkSwapchainKHR swapchain;
			std::vector<Texture*> swapchainTextures;
			Texture *swapchainDepth;
		};

		struct VkTexture {
			VkImage image;
			VkDeviceMemory memory;
			VkImageView view;
		};

		struct VkShaderStage {
			VkShaderModule shader;
			VkPipelineShaderStageCreateInfo pipeline;
		};

		template<u32 errorId, typename T = gc::Graphics>
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

			default:
				Log::error("VkResult: Fragmented pool");
				break;

			}

			return Log::throwError<T, errorId>(msg);
		}


		#define vkExtension(x) PFN_##x x = (PFN_##x) vkGetInstanceProcAddr(graphics.instance, #x); if (x == nullptr) oi::Log::throwError<oi::gc::VkGraphics, 0x0>("Couldn't get Vulkan extension");

		//TODO: Allocator
		#define allocator nullptr

		DEnum(VkTextureFormat, u32, Undefined = VK_FORMAT_UNDEFINED,

			RGBA8 = VK_FORMAT_B8G8R8A8_UNORM, RGB8 = VK_FORMAT_B8G8R8_UNORM, RG8 = VK_FORMAT_R8G8_UNORM, R8 = VK_FORMAT_R8_UNORM,
			RGBA8s = VK_FORMAT_B8G8R8A8_SNORM, RGB8s = VK_FORMAT_B8G8R8_SNORM, RG8s = VK_FORMAT_R8G8_SNORM, R8s = VK_FORMAT_R8_SNORM,
			RGBA8u = VK_FORMAT_B8G8R8A8_UINT, RGB8u = VK_FORMAT_B8G8R8_UINT, RG8u = VK_FORMAT_R8G8_UINT, R8u = VK_FORMAT_R8_UINT,
			RGBA8i = VK_FORMAT_B8G8R8A8_SINT, RGB8i = VK_FORMAT_B8G8R8_SINT, RG8i = VK_FORMAT_R8G8_SINT, R8i = VK_FORMAT_R8_SINT,

			RGBA16 = VK_FORMAT_R16G16B16A16_UNORM, RGB16 = VK_FORMAT_R16G16B16_UNORM, RG16 = VK_FORMAT_R16G16_UNORM, R16 = VK_FORMAT_R16_UNORM,
			RGBA16s = VK_FORMAT_R16G16B16A16_SNORM, RGB16s = VK_FORMAT_R16G16B16_SNORM, RG16s = VK_FORMAT_R16G16_SNORM, R16s = VK_FORMAT_R16_SNORM,
			RGBA16u = VK_FORMAT_R16G16B16A16_UINT, RGB16u = VK_FORMAT_R16G16B16_UINT, RG16u = VK_FORMAT_R16G16_UINT, R16u = VK_FORMAT_R16_UINT,
			RGBA16i = VK_FORMAT_R16G16B16A16_SINT, RGB16i = VK_FORMAT_R16G16B16_SINT, RG16i = VK_FORMAT_R16G16_SINT, R16i = VK_FORMAT_R16_SINT,
			RGBA16f = VK_FORMAT_R16G16B16A16_SFLOAT, RGB16f = VK_FORMAT_R16G16B16_SFLOAT, RG16f = VK_FORMAT_R16G16_SFLOAT, R16f = VK_FORMAT_R16_SFLOAT,

			RGBA32f = VK_FORMAT_R32G32B32A32_SFLOAT, RGB32f = VK_FORMAT_R32G32B32_SFLOAT, RG32f = VK_FORMAT_R32G32_SFLOAT, R32f = VK_FORMAT_R32_SFLOAT,
			RGBA32u = VK_FORMAT_R32G32B32A32_UINT, RGB32u = VK_FORMAT_R32G32B32_UINT, RG32u = VK_FORMAT_R32G32_UINT, R32u = VK_FORMAT_R32_UINT,
			RGBA32i = VK_FORMAT_R32G32B32A32_SINT, RGB32i = VK_FORMAT_R32G32B32_SINT, RG32i = VK_FORMAT_R32G32_SINT, R32i = VK_FORMAT_R32_SINT,

			D16 = VK_FORMAT_D16_UNORM, D32 = VK_FORMAT_D32_SFLOAT, D16S8 = VK_FORMAT_D16_UNORM_S8_UINT, D24S8 = VK_FORMAT_D24_UNORM_S8_UINT, D32S8 = VK_FORMAT_D32_SFLOAT_S8_UINT,

			sRGBA8 = VK_FORMAT_B8G8R8A8_SRGB, sRGB8 = VK_FORMAT_B8G8R8_SRGB, sRG8 = VK_FORMAT_R8G8_SRGB, sR8 = VK_FORMAT_R8_SRGB

		);

		DEnum(VkTextureUsage, u32, Undefined = VK_IMAGE_LAYOUT_UNDEFINED,

			Render_target = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, Render_depth = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			Shader_input_only = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, General = VK_IMAGE_LAYOUT_GENERAL

		);

		DEnum(VkShaderStageType, VkShaderStageFlagBits, Vertex_shader = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, Fragment_shader = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, Geometry_shader = VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT, Compute_shader = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);

	}
}