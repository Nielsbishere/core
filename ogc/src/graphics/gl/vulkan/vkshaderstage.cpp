#ifdef __VULKAN__

#include "graphics/graphics.h"
#include "graphics/gl/vulkan.h"
#include "graphics/objects/shader/shaderstage.h"
using namespace oi::gc;
using namespace oi;

ShaderStageExt &ShaderStage::getExtension() { return *ext; }

ShaderStage::~ShaderStage() {

	GraphicsExt &graphics = g->getExtension();

	if (ext->shader != VK_NULL_HANDLE)
		vkDestroyShaderModule(graphics.device, ext->shader, vkAllocator);

	g->dealloc<ShaderStage>(ext);
}

bool ShaderStage::init() {

	g->alloc<ShaderStage>(ext);

	GraphicsExt &graphics = g->getExtension();

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = info.code.size();
	createInfo.pCode = (uint32_t*) info.code.addr();

	vkCheck<0x0, ShaderStageExt>(vkCreateShaderModule(graphics.device, &createInfo, vkAllocator, &ext->shader), "Shader stage creation failed");
	vkName(graphics, ext->shader, VK_OBJECT_TYPE_SHADER_MODULE, getName());

	VkPipelineShaderStageCreateInfo &stageInfo = ext->pipeline;
	memset(&stageInfo, 0, sizeof(stageInfo));
	stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.stage = ShaderStageTypeExt(info.type.getName()).getValue();
	stageInfo.module = ext->shader;
	stageInfo.pName = "main";

	return true;
}

#endif