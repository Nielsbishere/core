#pragma once
#include "vulkan/vulkan.h"
#include "template/enum.h"

namespace oi {

	namespace gc {

		class Sampler;

		struct SamplerExt {

			typedef Sampler BaseType;

			VkSampler obj;
		};

		DEnum(SamplerWrappingExt, VkSamplerAddressMode,
			Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			Mirror_repeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
			Clamp_edge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			Clamp_border = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			Mirror_clamp_edge = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
		);

		SEnum(SamplerMinExt, VkFilter filter; VkSamplerMipmapMode mip;,
			Linear_mip = _({ VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR }),
			Nearest_mip = _({ VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST }),
			Linear = _({ VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR }),
			Nearest = _({ VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR }),
			Linear_mip_nearest = _({ VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST }),
			Nearest_mip_linear = _({ VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR })
		);

		DEnum(SamplerMagExt, VkFilter, Linear = VK_FILTER_LINEAR, Nearest = VK_FILTER_NEAREST);

	}

}