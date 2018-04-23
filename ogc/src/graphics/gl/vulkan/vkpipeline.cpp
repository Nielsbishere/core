#include "graphics/pipeline.h"
#include "graphics/pipelinestate.h"
#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/rendertarget.h"
using namespace oi::gc;
using namespace oi;

Pipeline::~Pipeline() {
	vkDestroyPipeline(g->getExtension().device, ext, allocator);
}

bool Pipeline::init(Graphics *g) {

	if (info.shader == nullptr)
		Log::throwError<Pipeline, 0x2>("Pipeline requires a shader");

	GraphicsExt &gext = g->getExtension();
	this->g = g;

	ShaderExt &shext = info.shader->getExtension();

	std::vector<VkPipelineShaderStageCreateInfo> stage(shext.stage.size());
	for (u32 i = 0; i < (u32)stage.size(); ++i)
		stage[i] = shext.stage[i]->pipeline;

	if (info.shader->getInfo().stage.size() > 1) {

		if (info.renderTarget == nullptr || info.pipelineState == nullptr)
			Log::throwError<Pipeline, 0x3>("Graphics pipeline requires a render target and/or pipeline state");

		//Dynamic viewport & scissor

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicInfo;
		memset(&dynamicInfo, 0, sizeof(dynamicInfo));

		dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicInfo.dynamicStateCount = 2;
		dynamicInfo.pDynamicStates = dynamicStates;

		VkPipelineViewportStateCreateInfo viewportInfo;
		memset(&viewportInfo, 0, sizeof(viewportInfo));

		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.scissorCount = 1;

		//Pipeline

		VkGraphicsPipelineCreateInfo pipelineInfo;
		memset(&pipelineInfo, 0, sizeof(pipelineInfo));

		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pDynamicState = &dynamicInfo;
		pipelineInfo.pViewportState = &viewportInfo;

		//Shader
		pipelineInfo.stageCount = (u32)stage.size();
		pipelineInfo.pStages = stage.data();
		pipelineInfo.layout = shext.layout;
		pipelineInfo.pVertexInputState = &shext.vertexInput;

		//PipelineState

		PipelineStateExt &psext = info.pipelineState->getExtension();

		pipelineInfo.pInputAssemblyState = &psext.assembler;
		pipelineInfo.pRasterizationState = &psext.rasterizer;
		pipelineInfo.pMultisampleState = &psext.multiSample;
		pipelineInfo.pDepthStencilState = &psext.depthStencil;
		pipelineInfo.pColorBlendState = &psext.blending;

		//RenderTarget

		RenderTargetExt &rtext = info.renderTarget->getExtension();

		pipelineInfo.renderPass = rtext.renderPass;
		pipelineInfo.subpass = 0;

		//Create the pipeline

		vkCheck<0x0, Pipeline>(vkCreateGraphicsPipelines(gext.device, nullptr, 1, &pipelineInfo, allocator, &ext), "Couldn't create graphics pipeline");

	} else {
	
		if (info.pipelineState != nullptr)
			Log::warn("Compute shader has a PipelineState which is set. This won't get used and it is advised that you set it to nullptr");

		if(info.renderTarget != nullptr)
			Log::warn("Compute shader has a RenderTarget which is set. This won't get used and it is advised that you set it to nullptr");

		//Create compute pipeline

		VkComputePipelineCreateInfo pipelineInfo;
		memset(&pipelineInfo, 0, sizeof(pipelineInfo));

		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = stage[0];
		pipelineInfo.layout = shext.layout;

		//Create the pipeline

		vkCheck<0x1, Pipeline>(vkCreateComputePipelines(gext.device, nullptr, 1, &pipelineInfo, allocator, &ext), "Couldn't create compute pipeline");

	}

	Log::println("Successfully created pipeline");
	return true;
}