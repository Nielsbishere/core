#ifdef __VULKAN__

#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/graphics.h"
#include "graphics/gbuffer.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

Shader::~Shader() {

	GraphicsExt &gext = g->getExtension();

	for (ShaderStage *stage : info.stage)
		g->destroy(stage);

	for (auto &sb : info.shaderRegister)
		if (sb.second != nullptr)
			g->destroy(sb.second);

	vkDestroyDescriptorPool(gext.device, ext.descriptorPool, allocator);
	vkDestroyDescriptorSetLayout(gext.device, ext.setLayout, allocator);
	vkDestroyPipelineLayout(gext.device, ext.layout, allocator);

}

void Shader::update() {

	if (changed) {

		//Update the entire descriptor set

		std::vector<VkWriteDescriptorSet> descriptorSet(info.registers.size());
		memset(descriptorSet.data(), 0, sizeof(VkWriteDescriptorSet) * descriptorSet.size());

		std::vector<VkDescriptorBufferInfo> buffers(info.buffer.size());
		memset(buffers.data(), 0, sizeof(VkDescriptorBufferInfo) * buffers.size());

		u32 i = 0, j = 0;

		for (ShaderRegister &reg : info.registers) {

			VkWriteDescriptorSet &descriptor = descriptorSet[i];

			descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor.dstSet = ext.descriptorSet;
			descriptor.dstBinding = i;
			descriptor.dstArrayElement = 0U;
			descriptor.descriptorType = ShaderRegisterTypeExt(reg.type.getName()).getValue();
			descriptor.descriptorCount = 1U;

			GraphicsResource *res = info.shaderRegister[reg.name];
			
			if (dynamic_cast<ShaderBuffer*>(res) != nullptr) {

				VkDescriptorBufferInfo *bufferInfo = buffers.data() + j;
				descriptor.pBufferInfo = bufferInfo;

				ShaderBuffer *shaderBuffer = (ShaderBuffer*) res;

				GBuffer *buf = shaderBuffer->getBuffer();

				if (buf == nullptr)
					Log::throwError<Shader, 0x0>("Shader mentions an invalid buffer");

				bufferInfo->buffer = buf != nullptr ? buf->getExtension().resource : nullptr;
				bufferInfo->range = (VkDeviceSize) shaderBuffer->getSize();

				++j;

			} else
				Log::throwError<Shader, 0x1>("Shader mentiones an invalid resource");

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

	//Set up bindings and attributes

	VkPipelineVertexInputStateCreateInfo &inputInfo = ext.vertexInput;
	memset(&inputInfo, 0, sizeof(inputInfo));

	auto &binding = ext.inputBuffer = std::vector<VkVertexInputBindingDescription>(info.section.size());
	auto &attribute = ext.inputAttribute = std::vector<VkVertexInputAttributeDescription>(info.var.size());
	
	for (u32 i = 0; i < binding.size(); ++i)
		binding[i] = { i, info.section[i].stride, info.section[i].perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX };

	for (u32 i = 0; i < attribute.size(); ++i)
		attribute[i] = { i, info.var[i].buffer, TextureFormatExt(info.var[i].type.getName()).getValue(), info.var[i].offset };

	inputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	inputInfo.vertexBindingDescriptionCount = (u32) binding.size();
	inputInfo.pVertexBindingDescriptions = binding.data();
	inputInfo.vertexAttributeDescriptionCount = (u32) attribute.size();
	inputInfo.pVertexAttributeDescriptions = attribute.data();

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

	vkCheck<0x1, VkShader>(vkCreateDescriptorSetLayout(gext.device, &descInfo, allocator, &ext.setLayout), "Couldn't create descriptor set layout");

	//Create pipeline layout

	VkPipelineLayoutCreateInfo layoutInfo;
	memset(&layoutInfo, 0, sizeof(layoutInfo));

	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1U;
	layoutInfo.pSetLayouts = &ext.setLayout;

	vkCheck<0x0, VkShader>(vkCreatePipelineLayout(gext.device, &layoutInfo, allocator, &ext.layout), "Couldn't create shader pipeline layout");

	//Create descriptor pool

	std::vector<VkDescriptorPoolSize> descriptorPool(orderedRegisters.size());

	i = 0;
	for (auto &reg : orderedRegisters) {

		VkDescriptorPoolSize &desc = descriptorPool[i];
		memset(&desc, 0, sizeof(desc));

		desc.descriptorCount = (u32)reg.second.size();
		desc.type = reg.first;

		++i;
	}

	VkDescriptorPoolCreateInfo poolInfo;
	memset(&poolInfo, 0, sizeof(poolInfo));

	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (u32) descriptorPool.size();
	poolInfo.pPoolSizes = descriptorPool.data();
	poolInfo.maxSets = 1U;

	vkCheck<0x2, VkShader>(vkCreateDescriptorPool(gext.device, &poolInfo, allocator, &ext.descriptorPool), "Couldn't create descriptor pool");

	//Create descriptor set

	VkDescriptorSetAllocateInfo setInfo;
	memset(&setInfo, 0, sizeof(setInfo));

	setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setInfo.descriptorPool = ext.descriptorPool;
	setInfo.descriptorSetCount = 1;
	setInfo.pSetLayouts = &ext.setLayout;

	vkCheck<0x3, VkShader>(vkAllocateDescriptorSets(gext.device, &setInfo, &ext.descriptorSet), "Couldn't create descriptor sets");

	//Create shader buffers

	info.shaderRegister.reserve(info.registers.size());

	for (auto &sb : info.buffer)
		set(sb.first, sb.second.allocate ? g->create(info.buffer[sb.first]) : nullptr);

	changed = true;

	return true;

}

#endif