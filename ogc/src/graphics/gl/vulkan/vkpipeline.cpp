#include "graphics/pipeline.h"
#include "graphics/pipelinestate.h"
#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/rendertarget.h"
#include "graphics/versionedtexture.h"
#include "graphics/meshbuffer.h"
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
		return Log::throwError<Pipeline, 0x2>("Pipeline requires a shader");

	GraphicsExt &gext = g->getExtension();

	ShaderExt &shext = info.shader->getExtension();

	std::vector<VkPipelineShaderStageCreateInfo> stage(shext.stage.size());
	for (u32 i = 0; i < (u32)stage.size(); ++i)
		stage[i] = shext.stage[i]->pipeline;

	if (info.shader->getInfo().stage.size() > 1) {

		if (info.renderTarget == nullptr || info.pipelineState == nullptr || info.meshBuffer == nullptr)
			return Log::throwError<Pipeline, 0x3>("Graphics pipeline requires a render target, pipeline state and mesh buffer");

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

		//PipelineState

		PipelineStateExt &psext = info.pipelineState->getExtension();
		
		VkPipelineRasterizationStateCreateInfo rasterizer = psext.rasterizer;
		VkPipelineInputAssemblyStateCreateInfo assembler = psext.assembler;

		rasterizer.polygonMode = FillModeExt(info.meshBuffer->getInfo().fillMode.getName()).getValue();
		assembler.topology = TopologyModeExt(info.meshBuffer->getInfo().topologyMode.getName()).getValue();

		if (!g->getExtension().pfeatures.wideLines)
			rasterizer.lineWidth = 1.f;

		pipelineInfo.pInputAssemblyState = &assembler;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &psext.multiSample;
		pipelineInfo.pDepthStencilState = &psext.depthStencil;
		pipelineInfo.pColorBlendState = &psext.blending;

		//MeshBuffer
		//Set up bindings and attributes

		const MeshBufferInfo meshBuffer = info.meshBuffer->getInfo();

		VkPipelineVertexInputStateCreateInfo inputInfo;
		memset(&inputInfo, 0, sizeof(inputInfo));

		u32 attributes = 0;

		for (auto &elem : meshBuffer.buffers)
			attributes += (u32) elem.size();

		auto binding = std::vector<VkVertexInputBindingDescription>(meshBuffer.buffers.size());
		auto attribute = std::vector<VkVertexInputAttributeDescription>(attributes);
		memset(attribute.data(), 0, sizeof(VkVertexInputAttributeDescription) * attribute.size());

		u32 i = 0;

		const ShaderInfo shinfo = info.shader->getInfo();

		for (auto &elem : meshBuffer.buffers) {

			binding[i] = { i, meshBuffer.vboStrides[i], VK_VERTEX_INPUT_RATE_VERTEX };

			u32 offset = 0;

			for (auto elem0 : elem) {

				String name = elem0.first;
				TextureFormat format = elem0.second;

				u32 j = 0;

				for (ShaderInput var : shinfo.inputs)
					if (var.name == name) {

						if (!Graphics::isCompatible(var.type, format))
							return Log::throwError<PipelineExt, 0x1>(String("Couldn't create pipeline; Shader vertex input type didn't match up with vertex input type; ") + info.shader->getName() + "'s " + var.name + " and " + info.meshBuffer->getName() + "'s " + name);

						break;
					}
					else ++j;

				if(j == (u32) shinfo.inputs.size())
					return Log::throwError<PipelineExt, 0x0>(String("Couldn't create pipeline; no match found in shader input from vertex input; ") + name);

				if(attribute[j].format != 0)
					return Log::throwError<PipelineExt, 0x0>(String("Couldn't create pipeline; vertex input (") + name + ") is already set. Don't use duplicate vertex inputs");

				attribute[j] = { j, i, TextureFormatExt(format.getName()).getValue(), offset };

				offset += Graphics::getFormatSize(format);

			}

			++i;
		}

		inputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		inputInfo.vertexBindingDescriptionCount = (u32)binding.size();
		inputInfo.pVertexBindingDescriptions = binding.data();
		inputInfo.vertexAttributeDescriptionCount = (u32)attribute.size();
		inputInfo.pVertexAttributeDescriptions = attribute.data();
		pipelineInfo.pVertexInputState = &inputInfo;

		//RenderTarget

		RenderTarget *rt = info.renderTarget;
		RenderTargetExt &rtext = rt->getExtension();

		pipelineInfo.renderPass = rtext.renderPass;
		pipelineInfo.subpass = 0;

		//Validate pipeline

		for (const ShaderOutput so : info.shader->getInfo().outputs) {

			if (so.id >= rt->getTargets())
				Log::throwError<Pipeline, 0x4>("Invalid pipeline; Shader referenced a shader output to an unknown output");
			
			if(!Graphics::isCompatible(so.type, rt->getTarget(so.id)->getFormat()))
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