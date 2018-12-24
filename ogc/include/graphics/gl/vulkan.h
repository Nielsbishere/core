#pragma once

#ifdef __VULKAN__

#include <vector>
#include "types/string.h"
#include "utils/log.h"
#include "template/enum.h"
#include "vulkan/vulkan.h"

namespace oi {
	
	namespace gc {
		
		class Texture;
		class Graphics;
		class CommandList;

		struct VkGraphics;

		class GBufferType;

		struct VkRenderTarget {

			VkRenderPass renderPass = VK_NULL_HANDLE;
			std::vector<VkFramebuffer> frameBuffer;

		};

		struct VkTexture {

			VkImage resource = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			VkImageView view = VK_NULL_HANDLE;
			VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	//Only applicable to manual transitions; like compute shader

		};

		struct VkGBuffer {

			std::vector<VkBuffer> resource;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			u32 gpuAlignment = 0;

			static bool isVersioned(GBufferType type);
			static bool isStaged(GBufferType type);
			static bool isCoherent(GBufferType type);

		};

		struct VkShaderStage {

			VkShaderModule shader = VK_NULL_HANDLE;
			VkPipelineShaderStageCreateInfo pipeline{};

		};

		struct VkShader {

			std::vector<VkShaderStage*> stage;

			VkPipelineLayout layout = VK_NULL_HANDLE;
			VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
			VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
			std::vector<VkDescriptorSet> descriptorSet;

		};

		struct VkCommandList {

			VkCommandPool pool = VK_NULL_HANDLE;
			std::vector<VkCommandBuffer> cmds;

			VkCommandBuffer &cmd(VkGraphics &g);

		};

		struct VkPipelineState {

			VkPipelineInputAssemblyStateCreateInfo assembler{};
			VkPipelineRasterizationStateCreateInfo rasterizer{};
			VkPipelineColorBlendAttachmentState blendState{};
			VkPipelineColorBlendStateCreateInfo blending{};
			VkPipelineDepthStencilStateCreateInfo depthStencil{};
			VkPipelineMultisampleStateCreateInfo multiSample{};

		};

		struct VkGraphics {

			VkInstance instance = VK_NULL_HANDLE;
			VkPhysicalDevice pdevice = VK_NULL_HANDLE;
			VkDevice device = VK_NULL_HANDLE;
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			VkQueue queue = VK_NULL_HANDLE;
			VkSwapchainKHR swapchain = VK_NULL_HANDLE;
			VkCommandPool pool = VK_NULL_HANDLE;

			VkPhysicalDeviceFeatures pfeatures{};
			VkPhysicalDeviceProperties pproperties{};
			VkPhysicalDeviceMemoryProperties pmemory{};

			VkColorSpaceKHR colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
			VkFormat colorFormat = VK_FORMAT_UNDEFINED;

			std::vector<VkFence> presentFence;
			std::vector<VkSemaphore> submitSemaphore, swapchainSemaphore;

			CommandList *stagingCmdList;
			std::vector<std::vector<VkGBuffer>> stagingBuffers;

			u32 current = 0, frames = 0;
			u32 queueFamilyIndex = u32_MAX;

			#ifdef __DEBUG__

				VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;

				#ifdef __WINDOWS__
					PFN_vkSetDebugUtilsObjectNameEXT debugNames = nullptr;
				#endif

			#endif
			
		};

		template<u32 errorId, typename T = VkGraphics>
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

		#define vkExtension(x) PFN_##x x = (PFN_##x) vkGetInstanceProcAddr(ext.instance, #x); if (x == nullptr) oi::Log::throwError<oi::gc::VkGraphics, 0x9>("Couldn't get Vulkan extension");

		template<typename T>
		void vkName(VkGraphics &g, T val, VkObjectType type, String name) {

			#ifdef __WINDOWS__

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

			#endif

		}

		//Reserved for allocation
		constexpr VkAllocationCallbacks *vkAllocator = nullptr;

		DEnum(VkTextureFormat, VkFormat, Undefined = VK_FORMAT_UNDEFINED,

			RGBA8 = VK_FORMAT_R8G8B8A8_UNORM, RGB8 = VK_FORMAT_R8G8B8_UNORM, RG8 = VK_FORMAT_R8G8_UNORM, R8 = VK_FORMAT_R8_UNORM,
			RGBA8s = VK_FORMAT_R8G8B8A8_SNORM, RGB8s = VK_FORMAT_R8G8B8_SNORM, RG8s = VK_FORMAT_R8G8_SNORM, R8s = VK_FORMAT_R8_SNORM,
			RGBA8u = VK_FORMAT_R8G8B8A8_UINT, RGB8u = VK_FORMAT_R8G8B8_UINT, RG8u = VK_FORMAT_R8G8_UINT, R8u = VK_FORMAT_R8_UINT,
			RGBA8i = VK_FORMAT_R8G8B8A8_SINT, RGB8i = VK_FORMAT_R8G8B8_SINT, RG8i = VK_FORMAT_R8G8_SINT, R8i = VK_FORMAT_R8_SINT,

			RGBA16 = VK_FORMAT_R16G16B16A16_UNORM, RGB16 = VK_FORMAT_R16G16B16_UNORM, RG16 = VK_FORMAT_R16G16_UNORM, R16 = VK_FORMAT_R16_UNORM,
			RGBA16s = VK_FORMAT_R16G16B16A16_SNORM, RGB16s = VK_FORMAT_R16G16B16_SNORM, RG16s = VK_FORMAT_R16G16_SNORM, R16s = VK_FORMAT_R16_SNORM,
			RGBA16u = VK_FORMAT_R16G16B16A16_UINT, RGB16u = VK_FORMAT_R16G16B16_UINT, RG16u = VK_FORMAT_R16G16_UINT, R16u = VK_FORMAT_R16_UINT,
			RGBA16i = VK_FORMAT_R16G16B16A16_SINT, RGB16i = VK_FORMAT_R16G16B16_SINT, RG16i = VK_FORMAT_R16G16_SINT, R16i = VK_FORMAT_R16_SINT,
			RGBA16f = VK_FORMAT_R16G16B16A16_SFLOAT, RGB16f = VK_FORMAT_R16G16B16_SFLOAT, RG16f = VK_FORMAT_R16G16_SFLOAT, R16f = VK_FORMAT_R16_SFLOAT,

			RGBA32f = VK_FORMAT_R32G32B32A32_SFLOAT, RGB32f = VK_FORMAT_R32G32B32_SFLOAT, RG32f = VK_FORMAT_R32G32_SFLOAT, R32f = VK_FORMAT_R32_SFLOAT,
			RGBA32u = VK_FORMAT_R32G32B32A32_UINT, RGB32u = VK_FORMAT_R32G32B32_UINT, RG32u = VK_FORMAT_R32G32_UINT, R32u = VK_FORMAT_R32_UINT,
			RGBA32i = VK_FORMAT_R32G32B32A32_SINT, RGB32i = VK_FORMAT_R32G32B32_SINT, RG32i = VK_FORMAT_R32G32_SINT, R32i = VK_FORMAT_R32_SINT,

			RGBA64f = VK_FORMAT_R64G64B64A64_SFLOAT, RGB64f = VK_FORMAT_R64G64B64_SFLOAT, RG64f = VK_FORMAT_R64G64_SFLOAT, R64f = VK_FORMAT_R64_SFLOAT,
			RGBA64u = VK_FORMAT_R64G64B64A64_UINT, RGB64u = VK_FORMAT_R64G64B64_UINT, RG64u = VK_FORMAT_R64G64_UINT, R64u = VK_FORMAT_R64_UINT,
			RGBA64i = VK_FORMAT_R64G64B64A64_SINT, RGB64i = VK_FORMAT_R64G64B64_SINT, RG64i = VK_FORMAT_R64G64_SINT, R64i = VK_FORMAT_R64_SINT,

			D16 = VK_FORMAT_D16_UNORM, D32 = VK_FORMAT_D32_SFLOAT, D16S8 = VK_FORMAT_D16_UNORM_S8_UINT, D24S8 = VK_FORMAT_D24_UNORM_S8_UINT, D32S8 = VK_FORMAT_D32_SFLOAT_S8_UINT,

			sRGBA8 = VK_FORMAT_R8G8B8A8_SRGB, sRGB8 = VK_FORMAT_R8G8B8_SRGB, sRG8 = VK_FORMAT_R8G8_SRGB, sR8 = VK_FORMAT_R8_SRGB,

			BGRA8 = VK_FORMAT_B8G8R8A8_UNORM, BGR8 = VK_FORMAT_B8G8R8_UNORM,
			BGRA8s = VK_FORMAT_B8G8R8A8_SNORM, BGR8s = VK_FORMAT_B8G8R8_SNORM,
			BGRA8u = VK_FORMAT_B8G8R8A8_UINT, BGR8u = VK_FORMAT_B8G8R8_UINT,
			BGRA8i = VK_FORMAT_B8G8R8A8_SINT, BGR8i = VK_FORMAT_B8G8R8_SINT,
			sBGRA8 = VK_FORMAT_B8G8R8A8_SRGB, sBGR8 = VK_FORMAT_B8G8R8_SRGB

		);

		DEnum(VkTextureUsage, VkImageLayout, Undefined = VK_IMAGE_LAYOUT_UNDEFINED,

			Render_target = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, Render_depth = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			Compute_target = VK_IMAGE_LAYOUT_GENERAL,
			Image = VK_IMAGE_LAYOUT_UNDEFINED

		);

		DEnum(VkTopologyMode, VkPrimitiveTopology, 
			
			Points = VK_PRIMITIVE_TOPOLOGY_POINT_LIST, Line = VK_PRIMITIVE_TOPOLOGY_LINE_LIST, Line_strip = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, Triangle = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, Triangle_strip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, Triangle_fan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
			Line_adj = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, Line_strip_adj = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY, Triangle_adj = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY, Triangle_strip_adj = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY

		);

		DEnum(VkFillMode, VkPolygonMode, Fill = VK_POLYGON_MODE_FILL, Line = VK_POLYGON_MODE_LINE, Point = VK_POLYGON_MODE_POINT);
		DEnum(VkCullMode, VkCullModeFlags, None = VK_CULL_MODE_NONE, Back = VK_CULL_MODE_BACK_BIT, Front = VK_CULL_MODE_FRONT_BIT);
		DEnum(VkWindMode, VkFrontFace, CCW = VK_FRONT_FACE_COUNTER_CLOCKWISE, CW = VK_FRONT_FACE_CLOCKWISE);

		DEnum(VkGBufferType, VkBufferUsageFlags, UBO = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, SSBO = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, IBO = VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VBO = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, CBO = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

		DEnum(VkShaderRegisterType, VkDescriptorType,
			UBO = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SSBO = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			Texture2D = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Image = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			Sampler = VK_DESCRIPTOR_TYPE_SAMPLER
		);


		#ifdef __RAYTRACING__
		
			DEnum(VkShaderStageType, VkShaderStageFlagBits,
	
				Compute_shader = VK_SHADER_STAGE_COMPUTE_BIT,
	
				Vertex_shader = VK_SHADER_STAGE_VERTEX_BIT,
				Fragment_shader = VK_SHADER_STAGE_FRAGMENT_BIT,
				Geometry_shader = VK_SHADER_STAGE_GEOMETRY_BIT,
				Tesselation_shader = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
				Tesselation_evaluation_shader = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
				
				Mesh_shader = VK_SHADER_STAGE_MESH_BIT_NV,
				Task_shader = VK_SHADER_STAGE_TASK_BIT_NV,
		
				Ray_gen_shader = VK_SHADER_STAGE_RAYGEN_BIT_NV,
				Any_hit_shader = VK_SHADER_STAGE_ANY_HIT_BIT_NV,
				Closest_hit_shader = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
				Miss_shader = VK_SHADER_STAGE_MISS_BIT_NV,
				Intersection_shader = VK_SHADER_STAGE_INTERSECTION_BIT_NV,
				Callable_shader = VK_SHADER_STAGE_CALLABLE_BIT_NV
			);
		
		#else

			DEnum(VkShaderStageType, VkShaderStageFlagBits,
	
				Compute_shader = VK_SHADER_STAGE_COMPUTE_BIT,
	
				Vertex_shader = VK_SHADER_STAGE_VERTEX_BIT,
				Fragment_shader = VK_SHADER_STAGE_FRAGMENT_BIT,
				Geometry_shader = VK_SHADER_STAGE_GEOMETRY_BIT,
				Tesselation_shader = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
				Tesselation_evaluation_shader = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
				
			);
			
		#endif

		DEnum(VkSamplerWrapping, VkSamplerAddressMode, 
			Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT, Mirror_repeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, 
			Clamp_edge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, Clamp_border = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 
			Mirror_clamp_edge = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
		);

		SEnum(VkSamplerMin, VkFilter filter; VkSamplerMipmapMode mip; ,
			Linear_mip = _({ VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR }),
			Nearest_mip = _({ VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST }),
			Linear = _({ VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR }),
			Nearest = _({ VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR }),
			Linear_mip_nearest = _({ VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST }),
			Nearest_mip_linear = _({ VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR })
		);

		DEnum(VkSamplerMag, VkFilter, Linear = VK_FILTER_LINEAR, Nearest = VK_FILTER_NEAREST);

	}
}

//Hashing for VkDescriptorType
namespace std {
	template<>
	struct hash<VkDescriptorType> {
		inline size_t operator()(const VkDescriptorType& type) const {
			return (size_t) type;
		}
	};
}


#endif