#ifdef __VULKAN__
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/model/meshbuffer.h"
#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/pipelinestate.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/shader/shaderstage.h"
using namespace oi::gc;
using namespace oi;

Pipeline::~Pipeline() {

	g->destroy(graphicsInfo.shader);
	g->destroy(graphicsInfo.meshBuffer);
	g->destroy(graphicsInfo.pipelineState);
	g->destroy(graphicsInfo.renderTarget);
	g->destroy(computeInfo.shader);

	for(Shader *shader : raytracingInfo.shaders)
		g->destroy(shader);

	g->destroy(raytracingInfo.meshBuffer);

	vkDestroyPipeline(g->getExtension().device, ext, vkAllocator);
}

bool Pipeline::initData() {

	if (raytracingInfo.shaders.size() == 0 && computeInfo.shader == nullptr && graphicsInfo.shader == nullptr)
		return Log::throwError<VkPipeline, 0x0>("Pipeline requires a shader");

	GraphicsExt &gext = g->getExtension();

	if (type == PipelineType::Graphics) {

		GraphicsPipelineInfo &info = graphicsInfo;

		if (info.renderTarget == nullptr || info.pipelineState == nullptr || info.meshBuffer == nullptr)
			return Log::throwError<VkPipeline, 0x1>("Graphics pipeline requires a render target, pipeline state and mesh buffer");

		ShaderExt &shext = info.shader->getExtension();

		std::vector<VkPipelineShaderStageCreateInfo> stage(shext.stage.size());
		for (u32 i = 0; i < (u32)stage.size(); ++i)
			stage[i] = shext.stage[i]->pipeline;

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

		VkPipelineColorBlendStateCreateInfo blending = psext.blending;
		std::vector<VkPipelineColorBlendAttachmentState> attachments(info.renderTarget->getInfo().targets);

		u32 j = (u32) attachments.size();

		for (u32 i = 0; i < j; ++i)
			attachments[i] = *blending.pAttachments;

		blending.attachmentCount = j;
		blending.pAttachments = attachments.data();

		pipelineInfo.pInputAssemblyState = &assembler;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &psext.multiSample;
		pipelineInfo.pDepthStencilState = &psext.depthStencil;
		pipelineInfo.pColorBlendState = &blending;

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

				String ename = elem0.first;
				TextureFormat format = elem0.second;

				j = 0;

				for (ShaderInput var : shinfo.inputs)
					if (var.name == ename) {

						if (!Graphics::isCompatible(var.type, format))
							return Log::throwError<VkPipeline, 0x2>(String("Couldn't create pipeline; Shader vertex input type didn't match up with vertex input type; ") + info.shader->getName() + "'s " + var.name + " and " + info.meshBuffer->getName() + "'s " + ename);

						break;
					}
					else ++j;

				if(j == (u32) shinfo.inputs.size())
					return Log::throwError<VkPipeline, 0x3>(String("Couldn't create pipeline; no match found in shader input from vertex input; ") + ename);

				if(attribute[j].format != 0)
					return Log::throwError<VkPipeline, 0x4>(String("Couldn't create pipeline; vertex input (") + ename + ") is already set. Don't use duplicate vertex inputs");

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
				Log::throwError<VkPipeline, 0x5>("Invalid pipeline; Shader referenced a shader output to an unknown output");
			
			if(!Graphics::isCompatible(so.type, rt->getTarget(so.id)->getFormat()))
				Log::throwError<VkPipeline, 0x6>("Invalid pipeline; Shader referenced an incompatible output format");

		}


		//Create the pipeline

		vkCheck<0x7, VkPipeline>(vkCreateGraphicsPipelines(gext.device, VK_NULL_HANDLE, 1, &pipelineInfo, vkAllocator, &ext), "Couldn't create graphics pipeline");

	} else if(type == PipelineType::Compute) {
	
		ComputePipelineInfo &info = computeInfo;

		ShaderExt &shext = info.shader->getExtension();

		//Create compute pipeline

		VkComputePipelineCreateInfo pipelineInfo;
		memset(&pipelineInfo, 0, sizeof(pipelineInfo));

		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = shext.stage[0]->pipeline;
		pipelineInfo.layout = shext.layout;

		//Create the pipeline

		vkCheck<0x8, VkPipeline>(vkCreateComputePipelines(gext.device, VK_NULL_HANDLE, 1, &pipelineInfo, vkAllocator, &ext), "Couldn't create compute pipeline");

	}
	else if (type == PipelineType::Raytracing) {
		#ifdef __RAYTRACING__ 

			if (!g->supports(GraphicsFeature::Raytracing))
				Log::throwError<VkGraphics, 0xA>("Couldn't create pipeline; raytracing isn't supported");

			RaytracingPipelineInfo &info = raytracingInfo;

			u32 stageCount = 0;

			for (Shader *shader : info.shaders)
				stageCount += (u32) shader->getInfo().stage.size();

			std::vector<VkPipelineShaderStageCreateInfo> stage(stageCount);

			std::vector<VkRayTracingShaderGroupCreateInfoNV> groups(info.shaders.size());
			memset(groups.data(), 0, groups.size() * sizeof(VkRayTracingPipelineCreateInfoNV));

			u32 i = 0, j = 0;

			for (Shader *shader : info.shaders) {

				u32 gen = VK_SHADER_UNUSED_NV, chit = VK_SHADER_UNUSED_NV, 
					ahit = VK_SHADER_UNUSED_NV, intersection = VK_SHADER_UNUSED_NV;

				for (ShaderStage *shaderStage : shader->getInfo().stage) {

					stage[i] = shaderStage->getExtension().pipeline;

					switch (shaderStage->getInfo().type.getValue()) {

					case ShaderStageType::Any_hit_shader.value:
						ahit = i;
						break;

					case ShaderStageType::Closest_hit_shader.value:
						chit = i;
						break;

					case ShaderStageType::Intersection_shader.value:
						intersection = i;
						break;

					default:
						gen = i;
						break;

					}

					++i;
				}

				VkRayTracingShaderGroupCreateInfoNV &group = groups[i];

				group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;

				if (gen != VK_SHADER_UNUSED_NV)
					group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
				else if (intersection != VK_SHADER_UNUSED_NV)
					group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
				else
					group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;

				group.generalShader = gen;
				group.anyHitShader = ahit;
				group.closestHitShader = chit;
				group.intersectionShader = intersection;

				++j;
			}

			//Create raytracing pipeline

			VkRayTracingPipelineCreateInfoNV pipelineInfo;
			memset(&pipelineInfo, 0, sizeof(pipelineInfo));

			pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
			pipelineInfo.layout = info.shaders[0]->getExtension().layout;
			pipelineInfo.stageCount = stageCount;
			pipelineInfo.pStages = stage.data();
			pipelineInfo.maxRecursionDepth = info.maxRecursionDepth;
			pipelineInfo.groupCount = (u32) groups.size();
			pipelineInfo.pGroups = groups.data();

			//Create the pipeline

			VkGraphics &graphics = g->getExtension();

			vkCheck<0x9, VkPipeline>(graphics.vkCreateRayTracingPipelinesNV(gext.device, VK_NULL_HANDLE, 1, &pipelineInfo, vkAllocator, &ext), "Couldn't create raytracing pipeline");

		#else

			Log::throwError<VkPipeline, 0xB>("Couldn't create pipeline; raytracing option isn't turned on");

		#endif
	}

	vkName(gext, ext, VK_OBJECT_TYPE_PIPELINE, getName());

	Log::println("Successfully created pipeline");
	return true;
}

#endif