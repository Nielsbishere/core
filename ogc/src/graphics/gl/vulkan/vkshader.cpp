#ifdef __VULKAN__

#include "graphics/graphics.h"
#include "graphics/objects/gbuffer.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/shader/shaderstage.h"
#include "graphics/objects/texture/sampler.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/texture/texturelist.h"
#include "graphics/objects/render/rendertarget.h"
using namespace oi::gc;
using namespace oi;

Shader::~Shader() {

	GraphicsExt &gext = g->getExtension();

	for (ShaderStage *stage : info.stage)
		g->destroy(stage);

	for (auto &sb : info.shaderRegister)
		if (sb.second != nullptr)
			g->destroyObject(sb.second);

	vkDestroyDescriptorPool(gext.device, ext.descriptorPool, vkAllocator);
	vkDestroyDescriptorSetLayout(gext.device, ext.setLayout, vkAllocator);
	vkDestroyPipelineLayout(gext.device, ext.layout, vkAllocator);

}

void Shader::update() {

	if (changed) {

		//Update the entire descriptor set

		std::vector<VkWriteDescriptorSet> descriptorSet(info.registers.size() * g->getBuffering());
		memset(descriptorSet.data(), 0, sizeof(VkWriteDescriptorSet) * descriptorSet.size());

		std::vector<VkDescriptorBufferInfo> buffers(info.buffer.size() * g->getBuffering());
		memset(buffers.data(), 0, sizeof(VkDescriptorBufferInfo) * buffers.size());

		u32 samplers = 0, images = 0, bufferCount = (u32) info.buffer.size();

		for (ShaderRegister &reg : info.registers) {
			if (reg.type == ShaderRegisterType::Sampler) ++samplers;
			else if (reg.type == ShaderRegisterType::Image || reg.type == ShaderRegisterType::Texture2D) images += reg.size;
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
			descriptor.dstBinding = reg.id;
			descriptor.dstArrayElement = 0U;
			descriptor.descriptorType = ShaderRegisterTypeExt(reg.type.getName()).getValue();
			descriptor.descriptorCount = reg.size;

			bool versioned = false;

			GraphicsResource *res = info.shaderRegister[reg.name];
			
			if (res->isType<ShaderBuffer>()) {

				if (reg.type != ShaderRegisterType::UBO && reg.type != ShaderRegisterType::SSBO)
					Log::throwError<VkShader, 0x1>("A ShaderBuffer has been placed on a register not meant for buffers");

				VkDescriptorBufferInfo *bufferInfo = buffers.data() + j;
				descriptor.pBufferInfo = bufferInfo;

				ShaderBuffer *shaderBuffer = (ShaderBuffer*) res;

				GBuffer *buf = shaderBuffer->getBuffer();

				if (buf == nullptr)
					Log::throwError<VkShader, 0x0>("Shader mentions an invalid buffer");

				std::vector<VkBuffer> resources = buf->getExtension().resource;

				if (resources.size() != 1 && buf != nullptr) {

					for (u32 z = 0; z < g->getBuffering(); ++z) {

						VkDescriptorBufferInfo *buffer = bufferInfo + z * bufferCount;
						buffer->range = (VkDeviceSize) shaderBuffer->getSize();
						buffer->buffer = resources[z];

						VkWriteDescriptorSet &descriptorz = descriptorSet[i + z * info.registers.size()];

						descriptorz = descriptor;
						descriptorz.pBufferInfo = buffer;
						descriptorz.dstSet = ext.descriptorSet[z];

					}

					versioned = true;

				} else versioned = false;

				bufferInfo->buffer = buf != nullptr ? resources[0] : VK_NULL_HANDLE;
				bufferInfo->range = (VkDeviceSize) shaderBuffer->getSize();

				++j;

			} else if(res->isType<Sampler>()){

				if (reg.type != ShaderRegisterType::Sampler)
					Log::throwError<VkShader, 0x2>("A Sampler has been placed on a register not meant for samplers");

				Sampler *samp = (Sampler*)res;

				VkDescriptorImageInfo *image = samplerInfo.data() + k;
				descriptor.pImageInfo = image;

				image->sampler = samp->getExtension();

				++k;

			} else if(res->isType<Texture>()) {

				if (reg.type != ShaderRegisterType::Texture2D || reg.size != 1)
					Log::throwError<VkShader, 0x3>("A Texture has been placed on a register not meant for textures or it has to use a TextureList");

				Texture *tex = (Texture*) res;

				VkDescriptorImageInfo *image = imageInfo.data() + l;
				descriptor.pImageInfo = image;

				image->imageView = tex->getExtension().view;
				image->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		//TODO: Change for Image

				++l;

			} else if (res->isType<TextureList>()) {

				if (reg.type != ShaderRegisterType::Texture2D || reg.size == 1)
					Log::throwError<VkShader, 0x4>("A TextureList has been placed on a register not meant for textures or it has to use a Texture");

				TextureList *tex = (TextureList*)res;

				if (tex->size() != reg.size)
					Log::throwError<VkShader, 0x5>("A TextureList had invalid size!");

				VkDescriptorImageInfo *image = imageInfo.data() + l;
				descriptor.pImageInfo = image;

				for (u32 z = 0; z < reg.size; ++z) {
					image->imageView = tex->get(z)->getExtension().view;
					image->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		//TODO: Change for Image
					++image;
				}

				l += reg.size;

			} else if (res->isType<VersionedTexture>()) {

				if (reg.type != ShaderRegisterType::Texture2D && reg.type != ShaderRegisterType::Image)
					Log::throwError<VkShader, 0x6>("A VersionedTexture has been placed on a register not meant for textures");

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
				Log::throwError<VkShader, 0x7>("Shader mentions an invalid resource");

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

bool Shader::initData() {

	GraphicsExt &gext = g->getExtension();

	//Set up stages

	ext.stage.resize(info.stage.size());

	for (u32 i = 0; i < (u32) ext.stage.size(); ++i)
		ext.stage[i] = &info.stage[i]->getExtension();

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

			u32 typeId = type.getValue();
			u32 typeFlag = typeId == 0 ? 0 : 1 << (typeId - 1);

			if (((u32)reg.access & typeFlag) == 0 && reg.access != ShaderAccessType::COMPUTE)
				continue;

			VkShaderStageType shaderType(type.getName());
			descInfo.stageFlags |= shaderType.getValue();

			if (reg.access == ShaderAccessType::COMPUTE)
				break;

		}

		orderedRegisters[ShaderRegisterTypeExt(reg.type.getName()).getValue()].add(reg);

		info.shaderRegister[reg.name] = nullptr;

		++i;
	}

	VkDescriptorSetLayoutCreateInfo descInfo;
	memset(&descInfo, 0, sizeof(descInfo));

	descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descInfo.bindingCount = (u32) descriptorSet.size();
	descInfo.pBindings = descriptorSet.data();

	vkCheck<0x8, VkShader>(vkCreateDescriptorSetLayout(gext.device, &descInfo, vkAllocator, &ext.setLayout), "Couldn't create descriptor set layout");
	vkName(gext, ext.setLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, getName() + " descriptor set layout");

	//Create pipeline layout

	VkPipelineLayoutCreateInfo layoutInfo;
	memset(&layoutInfo, 0, sizeof(layoutInfo));

	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1U;
	layoutInfo.pSetLayouts = &ext.setLayout;

	vkCheck<0x9, VkShader>(vkCreatePipelineLayout(gext.device, &layoutInfo, vkAllocator, &ext.layout), "Couldn't create shader pipeline layout");
	vkName(gext, ext.layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, getName() + " pipeline layout");

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
	poolInfo.poolSizeCount = (u32) descriptorPool.size();
	poolInfo.pPoolSizes = descriptorPool.data();
	poolInfo.maxSets = g->getBuffering();

	vkCheck<0xA, VkShader>(vkCreateDescriptorPool(gext.device, &poolInfo, vkAllocator, &ext.descriptorPool), "Couldn't create descriptor pool");
	vkName(gext, ext.descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, getName() + " descriptor pool");

	//Create descriptor set (versioned)

	VkDescriptorSetAllocateInfo setInfo;
	memset(&setInfo, 0, sizeof(setInfo));

	std::vector<VkDescriptorSetLayout> layouts(g->getBuffering());
	
	for (i = 0; i < g->getBuffering(); ++i)
		layouts[i] = ext.setLayout;

	setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setInfo.descriptorPool = ext.descriptorPool;
	setInfo.descriptorSetCount = g->getBuffering();
	setInfo.pSetLayouts = layouts.data();

	ext.descriptorSet.resize(g->getBuffering());

	vkCheck<0xB, VkShader>(vkAllocateDescriptorSets(gext.device, &setInfo, ext.descriptorSet.data()), "Couldn't create descriptor sets");

	for (u32 j = 0, k = (u32)ext.descriptorSet.size(); j < k; ++j)
		vkName(gext, ext.descriptorSet[j], VK_OBJECT_TYPE_DESCRIPTOR_SET, getName() + " descriptor set " + j);

	//Create shader buffers

	info.shaderRegister.reserve(info.registers.size());

	for (auto &sb : info.buffer) {

		ShaderBuffer *buffer = g->create(getName() + " " + sb.first, info.buffer[sb.first]);
		g->use(buffer);

		set(sb.first, buffer);
	}

	changed = true;

	return true;

}

#endif