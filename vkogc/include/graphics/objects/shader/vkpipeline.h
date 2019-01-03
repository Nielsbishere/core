#pragma once
#include "vulkan/vulkan.h"
#include "template/enum.h"

namespace oi {

	namespace gc {

		class Pipeline;

		struct PipelineExt {

			typedef Pipeline BaseType;

			VkPipeline obj;
		};

		#ifdef __RAYTRACING__

			DEnum(PipelineTypeExt, VkPipelineBindPoint, 
				Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS, 
				Compute = VK_PIPELINE_BIND_POINT_COMPUTE, 
				Raytracing = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
			);
		
		#else

			DEnum(PipelineTypeExt, VkPipelineBindPoint, 
				Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS, 
				Compute = VK_PIPELINE_BIND_POINT_COMPUTE
			);
			
		#endif

	}

}