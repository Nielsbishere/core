#ifdef __VULKAN__

#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

Shader::~Shader() {

	GraphicsExt &gext = g->getExtension();

	for (ShaderStage *stage : info.stage)
		g->destroy(stage);

	//vkDestroyDescriptorPool(gext.device, ext.descriptorPool, allocator);
	vkDestroyPipelineLayout(gext.device, ext.layout, allocator);

}

bool Shader::init() {

	GraphicsExt &gext = g->getExtension();

	ext.stage.resize(info.stage.size());

	for (u32 i = 0; i < (u32) ext.stage.size(); ++i)
		ext.stage[i] = &info.stage[i]->getExtension();

	VkPipelineVertexInputStateCreateInfo &inputInfo = ext.vertexInput;
	memset(&inputInfo, 0, sizeof(inputInfo));

	ext.binding = std::vector<VkVertexInputBindingDescription>(info.section.size());
	ext.attribute = std::vector<VkVertexInputAttributeDescription>(info.var.size());
	
	for (u32 i = 0; i < ext.binding.size(); ++i)
		ext.binding[i] = { i, info.section[i].stride, info.section[i].perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX };

	for (u32 i = 0; i < ext.attribute.size(); ++i)
		ext.attribute[i] = { i, info.var[i].buffer, TextureFormatExt(info.var[i].type.getName()).getValue(), info.var[i].offset };

	inputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	inputInfo.vertexBindingDescriptionCount = (u32) ext.binding.size();
	inputInfo.pVertexBindingDescriptions = ext.binding.data();				//Whoops; this won't be available
	inputInfo.vertexAttributeDescriptionCount = (u32) ext.attribute.size();
	inputInfo.pVertexAttributeDescriptions = ext.attribute.data();

	//TODO: setup constant & descriptor in ext

	/*VkDescriptorPoolCreateInfo poolInfo;
	memset(&poolInfo, 0, sizeof(poolInfo));

	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.*/

	VkPipelineLayoutCreateInfo layoutInfo;
	memset(&layoutInfo, 0, sizeof(layoutInfo));

	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = (u32) ext.descriptor.size();
	layoutInfo.pSetLayouts = ext.descriptor.data();
	layoutInfo.pushConstantRangeCount = (u32) ext.constant.size();
	layoutInfo.pPushConstantRanges = ext.constant.data();

	vkCheck<0x0, VkShader>(vkCreatePipelineLayout(gext.device, &layoutInfo, allocator, &ext.layout), "Couldn't create shader pipeline layout");

	return true;

}

#endif