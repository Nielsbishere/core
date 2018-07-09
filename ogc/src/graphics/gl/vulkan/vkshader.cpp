#ifdef __VULKAN__

#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/graphics.h"
#include "graphics/gbuffer.h"
#include "graphics/graphics.h"
#include "graphics/sampler.h"
#include "graphics/versionedtexture.h"
#include "graphics/rendertarget.h"
using namespace oi::gc;
using namespace oi;

Shader::~Shader() {

	GraphicsExt &gext = g->getExtension();

	for (ShaderStage *stage : info.stage)
		g->destroy(stage);

	for (auto &sb : info.shaderRegister)
		if (sb.second != nullptr)
			g->destroy(sb.second);

	vkDestroyDescriptorPool(gext.device, ext.descriptorPool, vkAllocator);
	vkDestroyDescriptorSetLayout(gext.device, ext.setLayout, vkAllocator);
	vkDestroyPipelineLayout(gext.device, ext.layout, vkAllocator);

}

void Shader::update() {

	if (changed) {

		//Update the entire descriptor set

		std::vector<VkWriteDescriptorSet> descriptorSet(info.registers.size() * g->getBuffering());
		memset(descriptorSet.data(), 0, sizeof(VkWriteDescriptorSet) * descriptorSet.size());

		std::vector<VkDescriptorBufferInfo> buffers(info.buffer.size());
		memset(buffers.data(), 0, sizeof(VkDescriptorBufferInfo) * buffers.size());

		u32 samplers = 0, images = 0;

		for (ShaderRegister &reg : info.registers) {
			if (reg.type == ShaderRegisterType::Sampler) ++samplers;
			else if (reg.type == ShaderRegisterType::Image || reg.type == ShaderRegisterType::Texture2D) ++images;
		}

		std::vector<VkDescriptorImageInfo> imageInfo(images * g->getBuffering());
		memset(imageInfo.data(), 0, sizeof(VkDescriptorImageInfo) * images * g->getBuffering());

		std::vector<VkDescriptorImageInfo> samplerInfo(samplers);
		memset(samplerInfo.data(), 0, sizeof(VkDescriptorImageInfo) * samplers);

		u32 i = 0, j = 0, k = 0, l = 0;

		for (ShaderRegister &reg : info.registers) {

			VkWriteDescriptorSet &descriptor = descriptorSet[i /* + 0 * info.registers.size() */];

			descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor.dstSet = ext.descriptorSet[0];
			descriptor.dstBinding = i;
			descriptor.dstArrayElement = 0U;
			descriptor.descriptorType = ShaderRegisterTypeExt(reg.type.getName()).getValue();
			descriptor.descriptorCount = 1U;

			bool versioned = false;

			GraphicsResource *res = info.shaderRegister[reg.name];
			
			if (dynamic_cast<ShaderBuffer*>(res) != nullptr) {

				if (reg.type != ShaderRegisterType::UBO && reg.type != ShaderRegisterType::SSBO)
					Log::throwError<Shader, 0x1>("A ShaderBuffer has been placed on a register not meant for SBOs");

				VkDescriptorBufferInfo *bufferInfo = buffers.data() + j;
				descriptor.pBufferInfo = bufferInfo;

				ShaderBuffer *shaderBuffer = (ShaderBuffer*) res;

				GBuffer *buf = shaderBuffer->getBuffer();

				if (buf == nullptr)
					Log::throwError<Shader, 0x0>("Shader mentions an invalid buffer");

				bufferInfo->buffer = buf != nullptr ? buf->getExtension().resource : nullptr;
				bufferInfo->range = (VkDeviceSize) shaderBuffer->getSize();

				++j;

			} else if(dynamic_cast<Sampler*>(res) != nullptr){

				if (reg.type != ShaderRegisterType::Sampler)
					Log::throwError<Shader, 0x2>("A Sampler has been placed on a register not meant for samplers");

				Sampler *samp = (Sampler*)res;

				VkDescriptorImageInfo *image = samplerInfo.data() + k;
				descriptor.pImageInfo = image;

				image->sampler = samp->getExtension();

				++k;

			} else if(dynamic_cast<Texture*>(res) != nullptr) {

				if (reg.type != ShaderRegisterType::Texture2D)
					Log::throwError<Shader, 0x3>("A Texture has been placed on a register not meant for textures");

				Texture *tex = (Texture*) res;

				VkDescriptorImageInfo *image = imageInfo.data() + l;
				descriptor.pImageInfo = image;

				image->imageView = tex->getExtension().view;
				image->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		//TODO: Change for Image

				++l;

			} else if (dynamic_cast<VersionedTexture*>(res) != nullptr) {

				if (reg.type != ShaderRegisterType::Texture2D)
					Log::throwError<Shader, 0x3>("A VersionedTexture has been placed on a register not meant for textures");

				VersionedTexture *tex = (VersionedTexture*)res;

				versioned = true;

				VkDescriptorImageInfo *image = imageInfo.data() + l;
				descriptor.pImageInfo = image;

				image->imageView = tex->getVersion(0)->getExtension().view;
				image->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		//TODO: Change for Image

				for (u32 z = 1; z < g->getBuffering(); ++z) {

					VkDescriptorImageInfo *image0 = image + z * images;
					image0->imageLayout = image->imageLayout;
					image0->imageView = tex->getVersion(z)->getExtension().view;

					VkWriteDescriptorSet &descriptorz = descriptorSet[i + z * info.registers.size()];

					descriptorz = descriptor;
					descriptorz.pImageInfo = image0;
					descriptorz.dstSet = ext.descriptorSet[z];

				}

				++l;

			} else
				Log::throwError<Shader, 0x4>("Shader mentions an invalid resource");

			if (!versioned) {

				for (u32 z = 1; z < g->getBuffering(); ++z) {

					VkWriteDescriptorSet &descriptorz = descriptorSet[i + z * info.registers.size()];

					descriptorz = descriptor;
					descriptorz.dstSet = ext.descriptorSet[z];

				}

			}

			++i;
		}

		vkUpdateDescriptorSets(g->getExtension().device, (u32) descriptorSet.size(), descriptorSet.data(), 0, nullptr);

		changed = false;

	}

}

bool Shader::init() {

	GraphicsExt &gext = g->getExtension();

	//Set up stages

	ext.stage.resize(info.stage.size());

	for (u32 i = 0; i < (u32) ext.stage.size(); ++i)
		ext.stage[i] = &info.stage[i]->getExtension();

	//Set up descriptors

	std::vector<VkDescriptorSetLayoutBinding> descriptorSet(info.registers.size());

	std::unordered_map<VkDescriptorType, std::vector<ShaderRegister>> orderedRegisters;

	u32 i = 0;
	for (auto &reg : info.registers) {

		VkDescriptorSetLayoutBinding &descInfo = descriptorSet[i];
		memset(&descInfo, 0, sizeof(descInfo));

		descInfo.binding = i;
		descInfo.descriptorCount = 1U;
		descInfo.descriptorType = ShaderRegisterTypeExt(info.registers[i].type.getName()).getValue();
		descInfo.pImmutableSamplers = nullptr;
		descInfo.stageFlags = ShaderRegisterAccessExt(info.registers[i].access.getName()).getValue();

		orderedRegisters[ShaderRegisterTypeExt(reg.type.getName()).getValue()].push_back(reg);

		info.shaderRegister[reg.name] = nullptr;

		++i;
	}

	VkDescriptorSetLayoutCreateInfo descInfo;
	memset(&descInfo, 0, sizeof(descInfo));

	descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descInfo.bindingCount = (u32) descriptorSet.size();
	descInfo.pBindings = descriptorSet.data();

	vkCheck<0x1, VkShader>(vkCreateDescriptorSetLayout(gext.device, &descInfo, vkAllocator, &ext.setLayout), "Couldn't create descriptor set layout");

	//Create pipeline layout

	VkPipelineLayoutCreateInfo layoutInfo;
	memset(&layoutInfo, 0, sizeof(layoutInfo));

	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1U;
	layoutInfo.pSetLayouts = &ext.setLayout;

	vkCheck<0x0, VkShader>(vkCreatePipelineLayout(gext.device, &layoutInfo, vkAllocator, &ext.layout), "Couldn't create shader pipeline layout");

	//Create descriptor pool

	std::vector<VkDescriptorPoolSize> descriptorPool(orderedRegisters.size());

	i = 0;
	for (auto &reg : orderedRegisters) {

		VkDescriptorPoolSize &desc = descriptorPool[i];
		memset(&desc, 0, sizeof(desc));

		desc.descriptorCount = (u32)reg.second.size() * g->getBuffering();
		desc.type = reg.first;

		++i;
	}

	VkDescriptorPoolCreateInfo poolInfo;
	memset(&poolInfo, 0, sizeof(poolInfo));

	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (u32) descriptorPool.size();
	poolInfo.pPoolSizes = descriptorPool.data();
	poolInfo.maxSets = g->getBuffering();

	vkCheck<0x2, VkShader>(vkCreateDescriptorPool(gext.device, &poolInfo, vkAllocator, &ext.descriptorPool), "Couldn't create descriptor pool");

	//Create descriptor set (versioned)

	VkDescriptorSetAllocateInfo setInfo;
	memset(&setInfo, 0, sizeof(setInfo));

	std::vector<VkDescriptorSetLayout> layouts(g->getBuffering());
	
	for (u32 i = 0; i < g->getBuffering(); ++i)
		layouts[i] = ext.setLayout;

	setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setInfo.descriptorPool = ext.descriptorPool;
	setInfo.descriptorSetCount = g->getBuffering();
	setInfo.pSetLayouts = layouts.data();

	ext.descriptorSet.resize(g->getBuffering());

	vkCheck<0x3, VkShader>(vkAllocateDescriptorSets(gext.device, &setInfo, ext.descriptorSet.data()), "Couldn't create descriptor sets");

	//Create shader buffers

	info.shaderRegister.reserve(info.registers.size());

	for (auto &sb : info.buffer) {

		ShaderBuffer *buffer = nullptr;

		if (sb.second.allocate) {
			buffer = g->create(getName() + " " + sb.first, info.buffer[sb.first]);
			g->use(buffer);
		}

		set(sb.first, buffer);
	}

	changed = true;

	return true;

}

#endif