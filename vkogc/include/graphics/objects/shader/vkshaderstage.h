#pragma once
#include "vulkan/vulkan.h"
#include "template/enum.h"

namespace oi {

	namespace gc {

		class ShaderStage;

		struct ShaderStageExt {

			typedef ShaderStage BaseType;

			VkShaderModule shader = VK_NULL_HANDLE;
			VkPipelineShaderStageCreateInfo pipeline{};

		};

		#ifdef __RAYTRACING__
		
			DEnum(ShaderStageTypeExt, VkShaderStageFlagBits,
	
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

			DEnum(ShaderStageTypeExt, VkShaderStageFlagBits,
	
				Compute_shader = VK_SHADER_STAGE_COMPUTE_BIT,
	
				Vertex_shader = VK_SHADER_STAGE_VERTEX_BIT,
				Fragment_shader = VK_SHADER_STAGE_FRAGMENT_BIT,
				Geometry_shader = VK_SHADER_STAGE_GEOMETRY_BIT,
				Tesselation_shader = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
				Tesselation_evaluation_shader = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
				
			);

		#endif

	}

}