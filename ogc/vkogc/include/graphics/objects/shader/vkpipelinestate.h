#pragma once
#include "vulkan/vulkan.h"
#include "template/enum.h"

namespace oi {

	namespace gc {

		class PipelineState;

		struct PipelineStateExt {

			typedef PipelineState BaseType;

			VkPipelineInputAssemblyStateCreateInfo assembler{};
			VkPipelineRasterizationStateCreateInfo rasterizer{};
			VkPipelineColorBlendAttachmentState blendState{};
			VkPipelineColorBlendStateCreateInfo blending{};
			VkPipelineDepthStencilStateCreateInfo depthStencil{};
			VkPipelineMultisampleStateCreateInfo multiSample{};

		};

		DEnum(TopologyModeExt, VkPrimitiveTopology,

			Points = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
			Line = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
			Line_strip = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
			Triangle = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			Triangle_strip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
			Triangle_fan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,

			Line_adj = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
			Line_strip_adj = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
			Triangle_adj = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
			Triangle_strip_adj = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY

		);

		DEnum(FillModeExt, VkPolygonMode, Fill = VK_POLYGON_MODE_FILL, Line = VK_POLYGON_MODE_LINE, Point = VK_POLYGON_MODE_POINT);
		DEnum(CullModeExt, VkCullModeFlags, None = VK_CULL_MODE_NONE, Back = VK_CULL_MODE_BACK_BIT, Front = VK_CULL_MODE_FRONT_BIT);
		DEnum(WindModeExt, VkFrontFace, CCW = VK_FRONT_FACE_COUNTER_CLOCKWISE, CW = VK_FRONT_FACE_CLOCKWISE);

	}

}