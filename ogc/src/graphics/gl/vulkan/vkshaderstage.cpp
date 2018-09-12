#ifdef __VULKAN__

#include "graphics/shaderstage.h"
#include "graphics/gl/vulkan.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

ShaderStage::~ShaderStage() {

	VkGraphics &graphics = g->getExtension();

	if (ext.shader != nullptr)
		vkDestroyShaderModule(graphics.device, ext.shader, vkAllocator);
}

bool ShaderStage::init() {

	VkGraphics &graphics = g->getExtension();

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = info.code.size();
	createInfo.pCode = (uint32_t*) info.code.addr();

	vkCheck<0x1, ShaderStage>(vkCreateShaderModule(graphics.device, &createInfo, vkAllocator, &ext.shader), "Shader stage creation failed");

	VkPipelineShaderStageCreateInfo &stageInfo = ext.pipeline;
	memset(&stageInfo, 0, sizeof(stageInfo));
	stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.stage = VkShaderStageType(info.type.getName()).getValue();
	stageInfo.module = ext.shader;
	stageInfo.pName = "main";

	return true;
}

#endif