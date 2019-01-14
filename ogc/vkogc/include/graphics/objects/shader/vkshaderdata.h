#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "template/enum.h"

namespace oi {

	namespace gc {

		class ShaderData;

		struct ShaderDataExt {

			typedef ShaderData BaseType;

			VkPipelineLayout layout = VK_NULL_HANDLE;
			VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
			VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
			std::vector<VkDescriptorSet> descriptorSet;

		};

		DEnum(ShaderRegisterTypeExt, VkDescriptorType,
			UBO = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			SSBO = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			Texture2D = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			Image = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			Sampler = VK_DESCRIPTOR_TYPE_SAMPLER
		);

	}
}

//Hashing for VkDescriptorType
namespace std {
	template<>
	struct hash<VkDescriptorType> {
		inline size_t operator()(const VkDescriptorType& type) const {
			return (size_t)type;
		}
	};
}