#include <cmath>
#include "graphics/vulkan.h"
#include "graphics/objects/shader/pipelinestate.h"
#include "graphics/objects/shader/vkpipelinestate.h"
using namespace oi::gc;
using namespace oi;

PipelineState::~PipelineState() {
	g->dealloc(ext);
}

PipelineStateExt &PipelineState::getExtension() { return *ext; }

bool PipelineState::init() {

	g->alloc(ext);

	if (std::pow(2, (u32) std::log2(info.samples)) != info.samples)
		return Log::throwError<PipelineStateExt, 0x0>("PipelineState creation failed; sample count has to be base2");

	//Assembler

	VkPipelineInputAssemblyStateCreateInfo &assembler = ext->assembler;
	memset(&assembler, 0, sizeof(assembler));

	assembler.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembler.primitiveRestartEnable = VK_FALSE;

	//Rasterizer

	VkPipelineRasterizationStateCreateInfo &rasterizer = ext->rasterizer;
	memset(&rasterizer, 0, sizeof(rasterizer));

	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.lineWidth = info.lineWidth;
	rasterizer.cullMode = CullModeExt(info.cullMode.getName()).getValue();
	rasterizer.frontFace = WindModeExt(info.windMode.getName()).getValue();

	//Multi sampling

	VkPipelineMultisampleStateCreateInfo &multiSample = ext->multiSample;
	memset(&multiSample, 0, sizeof(multiSample));

	multiSample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSample.sampleShadingEnable = VK_FALSE;
	multiSample.rasterizationSamples = (VkSampleCountFlagBits) info.samples;
	
	//Depth

	VkPipelineDepthStencilStateCreateInfo &depthStencil = ext->depthStencil;
	memset(&depthStencil, 0, sizeof(depthStencil));
	
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = (info.depthMode.getValue() & 0x1) != 0;
	depthStencil.depthWriteEnable = (info.depthMode.getValue() & 0x2) != 0;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.front = {};
	depthStencil.back = {};

	//Blending

	VkPipelineColorBlendAttachmentState &blendState = ext->blendState;
	memset(&blendState, 0, sizeof(blendState));

	blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendState.blendEnable = info.blendMode == BlendMode::Off ? VK_FALSE : VK_TRUE;


	switch (info.blendMode.getValue()) {

	case BlendMode::Add.value:

		blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendState.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendState.colorBlendOp = VK_BLEND_OP_ADD;
		blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendState.alphaBlendOp = VK_BLEND_OP_ADD;

		break;

	case BlendMode::Subtract.value:

		blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendState.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendState.colorBlendOp = VK_BLEND_OP_SUBTRACT;
		blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendState.alphaBlendOp = VK_BLEND_OP_ADD;

		break;

	case BlendMode::Alpha.value:

		blendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendState.colorBlendOp = VK_BLEND_OP_ADD;
		blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendState.alphaBlendOp = VK_BLEND_OP_ADD;

		break;

	default:
		break;

	}

	VkPipelineColorBlendStateCreateInfo &blending = ext->blending;
	memset(&blending, 0, sizeof(blending));

	blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blending.attachmentCount = 1;
	blending.pAttachments = &blendState;

	return true;
}