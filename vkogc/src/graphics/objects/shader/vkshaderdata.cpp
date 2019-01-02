#ifdef __VULKAN__
#include "graphics/graphics.h"
#include "graphics/generic.h"
#include "graphics/vulkan.h"
#include "graphics/objects/shader/shaderdata.h"
#include "graphics/objects/texture/sampler.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/texture/texturelist.h"
using namespace oi::gc;

void ShaderData::destroyData() {

	GraphicsExt &gext = g->getExtension();

	vkDestroyDescriptorPool(gext.device, ext->descriptorPool, vkAllocator);
	vkDestroyDescriptorSetLayout(gext.device, ext->setLayout, vkAllocator);
	vkDestroyPipelineLayout(gext.device, ext->layout, vkAllocator);

	g->dealloc<ShaderData>(ext);

}

void ShaderData::requestUpdate() {
	changed.clear(true);
}

void ShaderData::update() {

	GraphicsExt &graphics = g->getExtension();
	u32 frame = graphics.current;

	//Update the descriptor set for this frame

	if (changed[frame]) {

		changed[frame] = false;

		std::vector<VkWriteDescriptorSet> descriptorSet(info.registers.size());
		memset(descriptorSet.data(), 0, sizeof(VkWriteDescriptorSet) * descriptorSet.size());

		std::vector<VkDescriptorBufferInfo> buffers(info.buffer.size());
		memset(buffers.data(), 0, sizeof(VkDescriptorBufferInfo) * buffers.size());

		u32 samplers = 0, images = 0;

		for (ShaderRegister &reg : info.registers) {
			if (reg.type == ShaderRegisterType::Sampler) ++samplers;
			else if (reg.type == ShaderRegisterType::Image || reg.type == ShaderRegisterType::Texture2D) images += reg.size;
		}

		std::vector<VkDescriptorImageInfo> imageInfo(images);
		memset(imageInfo.data(), 0, sizeof(VkDescriptorImageInfo) * images);

		std::vector<VkDescriptorImageInfo> samplerInfo(samplers);
		memset(samplerInfo.data(), 0, sizeof(VkDescriptorImageInfo) * samplers);

		u32 bufferId = 0, samplerId = 0, imageId = 0;

		for (u32 regId = 0, regs = (u32)info.registers.size(); regId < regs; ++regId) {

			ShaderRegister &reg = info.registers[regId];
			VkWriteDescriptorSet &descriptor = descriptorSet[regId];

			descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor.dstSet = ext->descriptorSet[frame];
			descriptor.dstBinding = reg.id;
			descriptor.dstArrayElement = 0U;
			descriptor.descriptorType = ShaderRegisterTypeExt(reg.type.getName()).getValue();
			descriptor.descriptorCount = reg.size;

			GraphicsResource *res = info.shaderData[reg.name];

			if (res == nullptr)
				Log::throwError<ShaderDataExt, 0x7>("Shader mentions an invalid resource");

			if (res->isType<ShaderBuffer>() || res->isType<GPUBuffer>()) {

				if (reg.type != ShaderRegisterType::UBO && reg.type != ShaderRegisterType::SSBO)
					Log::throwError<ShaderDataExt, 0x1>("A ShaderBuffer has been placed on a register not meant for buffers");

				VkDescriptorBufferInfo *bufferInfo = buffers.data() + bufferId;
				descriptor.pBufferInfo = bufferInfo;

				GPUBuffer *buf = res->isType<ShaderBuffer>() ? ((ShaderBuffer*)res)->getBuffer() : (GPUBuffer*)res;

				if (buf == nullptr)
					Log::throwError<ShaderDataExt, 0x0>("Shader mentions an invalid buffer");

				std::vector<VkBuffer> resources = buf->getExtension().resource;

				bufferInfo->buffer = resources[frame % resources.size()];
				bufferInfo->range = (VkDeviceSize)buf->getSize();
				++bufferId;

			} else if (res->isType<Sampler>()) {

				if (reg.type != ShaderRegisterType::Sampler)
					Log::throwError<ShaderDataExt, 0x2>("A Sampler has been placed on a register not meant for samplers");

				Sampler *samp = (Sampler*)res;

				VkDescriptorImageInfo *image = samplerInfo.data() + samplerId;
				descriptor.pImageInfo = image;

				image->sampler = samp->getExtension().obj;
				++samplerId;

			} else if (res->isType<Texture>() || res->isType<VersionedTexture>()) {

				if ((reg.type != ShaderRegisterType::Texture2D && reg.type != ShaderRegisterType::Image) || reg.size != 1)
					Log::throwError<ShaderDataExt, 0x3>("A Texture has been placed on a register not meant for textures or it has to use a TextureList");

				if (reg.type == ShaderRegisterType::Image && res->isType<Texture>())
					Log::throwError<ShaderDataExt, 0x6>("A Texture has been placed on a register not meant for textures");

				Texture *tex = res->isType<Texture>() ? (Texture*)res : ((VersionedTexture*)res)->getVersion(frame);

				if (tex->getSize() == Vec2u())
					Log::throwError<ShaderDataExt, 0xD>("Uninitialized RenderTargets can't be forwarded to Shader into a Texture register, use RenderTarget::resize to initialize it or set a valid resolution");

				VkDescriptorImageInfo *image = imageInfo.data() + imageId;
				descriptor.pImageInfo = image;

				image->imageView = tex->getExtension().view;
				image->imageLayout = reg.type == ShaderRegisterType::Image ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				++imageId;

			} else if (res->isType<TextureList>()) {

				if (reg.type != ShaderRegisterType::Texture2D || reg.size == 1)
					Log::throwError<ShaderDataExt, 0x4>("A TextureList has been placed on a register not meant for textures or it has to use a Texture");

				TextureList *textures = (TextureList*)res;

				if (textures->size() != reg.size)
					Log::throwError<ShaderDataExt, 0x5>("A TextureList had invalid size!");

				VkDescriptorImageInfo *image = imageInfo.data() + imageId;
				descriptor.pImageInfo = image;

				for (u32 z = 0; z < reg.size; ++z) {

					TextureObject *texObj = textures->get(z);
					Texture *tex = texObj->isType<VersionedTexture>() ? ((VersionedTexture*)texObj)->getVersion(frame) : (Texture*)texObj;

					if (tex->getSize() == Vec2u())
						Log::throwError<ShaderDataExt, 0xE>("Uninitialized RenderTargets can't be forwarded to Shader into a TextureList register, use RenderTarget::resize to initialize it or set a valid resolution");

					image->imageView = tex->getExtension().view;
					image->imageLayout = reg.type == ShaderRegisterType::Image ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					++image;
				}

				imageId += reg.size;

			} else
				Log::throwError<ShaderDataExt, 0xC>("Shader mentions an invalid resource type");
		}

		vkUpdateDescriptorSets(g->getExtension().device, (u32)descriptorSet.size(), descriptorSet.data(), 0, nullptr);
	}
}

ShaderDataExt &ShaderData::getExtension() { return *ext; }

bool ShaderData::initData() {

	g->alloc<ShaderData>(ext);

	GraphicsExt &gext = g->getExtension();

	//Set up descriptors

	std::vector<VkDescriptorSetLayoutBinding> descriptorSet(info.registers.size());

	struct OrderedRegister {

		std::vector<ShaderRegister> reg;
		u32 count = 0;

		void add(ShaderRegister sreg) {
			reg.push_back(sreg);
			count += sreg.size;
		}

	};

	std::unordered_map<VkDescriptorType, OrderedRegister> orderedRegisters;

	u32 i = 0;
	for (auto &reg : info.registers) {

		VkDescriptorSetLayoutBinding &descInfo = descriptorSet[i];
		memset(&descInfo, 0, sizeof(descInfo));

		descInfo.binding = reg.id;
		descInfo.descriptorCount = reg.size;
		descInfo.descriptorType = ShaderRegisterTypeExt(info.registers[i].type.getName()).getValue();
		descInfo.pImmutableSamplers = nullptr;
		descInfo.stageFlags = 0;

		auto values = ShaderStageType::getValues();

		for (ShaderStageType type : values) {

			u32 tid = type.getValue();
			u32 typeFlag = tid == 0 ? 0 : 1 << (tid - 1);

			if (((u32)reg.access & typeFlag) == 0 && reg.access != ShaderAccessType::COMPUTE)
				continue;

			ShaderStageTypeExt shaderType(type.getName());
			descInfo.stageFlags |= shaderType.getValue();

			if (reg.access == ShaderAccessType::COMPUTE)
				break;

		}

		orderedRegisters[ShaderRegisterTypeExt(reg.type.getName()).getValue()].add(reg);

		++i;
	}

	VkDescriptorSetLayoutCreateInfo descInfo;
	memset(&descInfo, 0, sizeof(descInfo));

	descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descInfo.bindingCount = (u32)descriptorSet.size();
	descInfo.pBindings = descriptorSet.data();

	vkCheck<0x8, ShaderDataExt>(vkCreateDescriptorSetLayout(gext.device, &descInfo, vkAllocator, &ext->setLayout), "Couldn't create descriptor set layout");
	vkName(gext, ext->setLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, getName() + " descriptor set layout");

	//Create pipeline layout

	VkPipelineLayoutCreateInfo layoutInfo;
	memset(&layoutInfo, 0, sizeof(layoutInfo));

	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1U;
	layoutInfo.pSetLayouts = &ext->setLayout;

	vkCheck<0x9, ShaderDataExt>(vkCreatePipelineLayout(gext.device, &layoutInfo, vkAllocator, &ext->layout), "Couldn't create shader pipeline layout");
	vkName(gext, ext->layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, getName() + " pipeline layout");

	//Create descriptor pool

	std::vector<VkDescriptorPoolSize> descriptorPool(orderedRegisters.size());

	i = 0;
	for (auto &reg : orderedRegisters) {

		VkDescriptorPoolSize &desc = descriptorPool[i];
		memset(&desc, 0, sizeof(desc));

		desc.descriptorCount = (u32)reg.second.count * g->getBuffering();
		desc.type = reg.first;

		++i;
	}

	VkDescriptorPoolCreateInfo poolInfo;
	memset(&poolInfo, 0, sizeof(poolInfo));

	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (u32)descriptorPool.size();
	poolInfo.pPoolSizes = descriptorPool.data();
	poolInfo.maxSets = g->getBuffering();

	vkCheck<0xA, ShaderDataExt>(vkCreateDescriptorPool(gext.device, &poolInfo, vkAllocator, &ext->descriptorPool), "Couldn't create descriptor pool");
	vkName(gext, ext->descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, getName() + " descriptor pool");

	//Create descriptor set (versioned)

	VkDescriptorSetAllocateInfo setInfo;
	memset(&setInfo, 0, sizeof(setInfo));

	std::vector<VkDescriptorSetLayout> layouts(g->getBuffering());

	for (i = 0; i < g->getBuffering(); ++i)
		layouts[i] = ext->setLayout;

	setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setInfo.descriptorPool = ext->descriptorPool;
	setInfo.descriptorSetCount = g->getBuffering();
	setInfo.pSetLayouts = layouts.data();

	ext->descriptorSet.resize(g->getBuffering());

	vkCheck<0xB, ShaderDataExt>(vkAllocateDescriptorSets(gext.device, &setInfo, ext->descriptorSet.data()), "Couldn't create descriptor sets");

	for (u32 j = 0, k = (u32)ext->descriptorSet.size(); j < k; ++j)
		vkName(gext, ext->descriptorSet[j], VK_OBJECT_TYPE_DESCRIPTOR_SET, getName() + " descriptor set " + j);

	return true;
}

#endif