#include "graphics/pipeline.h"
#include "graphics/pipelinestate.h"
#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/rendertarget.h"
using namespace oi::gc;
using namespace oi;

Pipeline::~Pipeline() {

	g->destroy(info.pipelineState);
	g->destroy(info.shader);
	g->destroy(info.renderTarget);

	vkDestroyPipeline(g->getExtension().device, ext, vkAllocator);
}

bool Pipeline::init() {

	if (info.shader == nullptr)
		Log::throwError<Pipeline, 0x2>("Pipeline requires a shader");

	GraphicsExt &gext = g->getExtension();

	ShaderExt &shext = info.shader->getExtension();

	std::vector<VkPipelineShaderStageCreateInfo> stage(shext.stage.size());
	for (u32 i = 0; i < (u32)stage.size(); ++i)
		stage[i] = shext.stage[i]->pipeline;

	if (info.shader->getInfo().stage.size() > 1) {

		if (info.renderTarget == nullptr || info.pipelineState == nullptr)
			Log::throwError<Pipeline, 0x3>("Graphics pipeline requires a render target and/or pipeline state");

		//Init viewport and scissor

		VkPipelineViewportStateCreateInfo viewportInfo;
		memset(&viewportInfo, 0, sizeof(viewportInfo));

		VkViewport viewport;
		memset(&viewport, 0, sizeof(viewport));

		viewport.width = (float) info.renderTarget->getSize().x;
		viewport.height = (float) info.renderTarget->getSize().y;
		viewport.maxDepth = 1;

		VkRect2D scissor;
		memset(&scissor, 0, sizeof(scissor));

		scissor.extent = { (u32) viewport.width, (u32) viewport.height };

		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = &viewport;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = &scissor;

		//Pipeline

		VkGraphicsPipelineCreateInfo pipelineInfo;
		memset(&pipelineInfo, 0, sizeof(pipelineInfo));

		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pDynamicState = nullptr;									//No dynamic states
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

		RenderTarget *rt = info.renderTarget;
		RenderTargetExt &rtext = rt->getExtension();

		pipelineInfo.renderPass = rtext.renderPass;
		pipelineInfo.subpass = 0;

		//Validate pipeline

		for (const ShaderOutput so : info.shader->getInfo().output) {

			if (so.id >= rt->getTargets() - 1U)
				Log::throwError<Pipeline, 0x4>("Invalid pipeline; Shader referenced a shader output to an unknown output");
			
			if(!Graphics::isCompatible(so.type, rt->getTarget(so.id + 1, 0)->getFormat()))
				Log::throwError<Pipeline, 0x5>("Invalid pipeline; Shader referenced an incompatible output format");

		}


		//Create the pipeline

		vkCheck<0x0, Pipeline>(vkCreateGraphicsPipelines(gext.device, nullptr, 1, &pipelineInfo, vkAllocator, &ext), "Couldn't create graphics pipeline");

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

		vkCheck<0x1, Pipeline>(vkCreateComputePipelines(gext.device, nullptr, 1, &pipelineInfo, vkAllocator, &ext), "Couldn't create compute pipeline");

	}

	Log::println("Successfully created pipeline");
	return true;
}