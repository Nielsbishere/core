#include "graphics/graphics.h"
#include "graphics/generic.h"
#include "graphics/vulkan.h"
#include "graphics/objects/gpubuffer.h"
#include "graphics/objects/render/commandlist.h"
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/render/drawlist.h"
#include "graphics/objects/shader/computelist.h"
#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/shaderdata.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/model/mesh.h"
#include "graphics/objects/texture/vktexture.h"
#include "graphics/objects/render/vkrendertarget.h"
#include "graphics/objects/render/vkcommandlist.h"
#include "graphics/objects/shader/vkpipeline.h"
#include "graphics/objects/shader/vkshaderdata.h"
using namespace oi::gc;
using namespace oi;

CommandList::~CommandList() {
	vkFreeCommandBuffers(g->getExtension().device, ext->pool, (u32) ext->cmds.size(), ext->cmds.begin());
	g->dealloc(ext);
}

CommandListExt &CommandList::getExtension() { return *ext; }

VkCommandBuffer &CommandListExt::cmd(GraphicsExt &g) { return cmds[g.frameId % cmds.size()]; }
#define ext_cmd ext->cmd(g->getExtension())

void CommandList::begin() {

	VkCommandBufferBeginInfo beginInfo;
	memset(&beginInfo, 0, sizeof(beginInfo));

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(ext_cmd, &beginInfo);
}

void CommandList::begin(RenderTarget *target, RenderTargetClear clear) {

	u32 frameId = g->getExtension().frameId;
	u32 swapchainId = g->getExtension().swapchainId;
	u32 rid = target->isOwned() ? frameId : swapchainId;

	if (target->isComputeTarget()) {

		VkImageMemoryBarrier imageBarrier;
		memset(&imageBarrier, 0, sizeof(imageBarrier));

		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		VkImageMemoryBarrier *barriers = new VkImageMemoryBarrier[target->getTargets()];

		for (u32 i = 0; i < target->getTargets(); ++i) {
			Texture *tex = target->getTarget(i)->getVersion(rid);
			(barriers[i] = imageBarrier).image = tex->getExtension().resource;
		}

		vkCmdPipelineBarrier(ext_cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, target->getTargets(), barriers);
		delete[] barriers;

		return;
	}

	RenderTargetExt &rtext = target->getExtension();

	VkRenderPassBeginInfo beginInfo;
	memset(&beginInfo, 0, sizeof(beginInfo));

	bool depthTarget = target->getDepth() != nullptr;

	u32 ctargets = target->getTargets();
	u32 targets = ctargets + depthTarget;

	Array<VkClearValue> clearValue(targets);

	for (u32 i = 0; i < targets; ++i) {

		bool isDepth = depthTarget && i == ctargets;

		VkClearValue &cl = clearValue[i];
		TextureFormat format = isDepth ? target->getDepth()->getFormat() : target->getTarget(i)->getFormat();

		if (isDepth) {
			cl.depthStencil.depth = clear.depthClear;
			cl.depthStencil.stencil = clear.stencilClear;
		} else {

			Vec4d color = g->convertColor(clear.colorClear, format);

			TextureFormatStorage storedFormat = g->getFormatStorage(format);

			if (storedFormat == TextureFormatStorage::INT)
				*(Vec4i*)cl.color.int32 = Vec4i(color);
			else if (storedFormat == TextureFormatStorage::UINT)
				*(Vec4u*)cl.color.uint32 = Vec4u(color);
			else
				*(Vec4*)cl.color.float32 = Vec4(color);

		}
	}

	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderArea.extent = { target->getSize().x, target->getSize().y };
	beginInfo.renderPass = rtext.renderPass;
	beginInfo.framebuffer = rtext.frameBuffer[rid];
	beginInfo.clearValueCount = (u32)clearValue.size();
	beginInfo.pClearValues = clearValue.begin();

	vkCmdBeginRenderPass(ext_cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

}

void CommandList::end(RenderTarget *target) {
	if (!target->isComputeTarget()) {
		vkCmdEndRenderPass(ext_cmd);
	} else {

		VkImageMemoryBarrier imageBarrier;
		memset(&imageBarrier, 0, sizeof(imageBarrier));

		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		VkImageMemoryBarrier *barriers = new VkImageMemoryBarrier[target->getTargets()];

		u32 frameId = g->getExtension().frameId;
		u32 swapchainId = g->getExtension().swapchainId;
		u32 rid = target->isOwned() ? frameId : swapchainId;

		for (u32 i = 0; i < target->getTargets(); ++i) {
			Texture *tex = target->getTarget(i)->getVersion(rid);
			(barriers[i] = imageBarrier).image = tex->getExtension().resource;
		}

		vkCmdPipelineBarrier(ext_cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, target->getTargets(), barriers);
		delete[] barriers;
	}
}

void CommandList::end() {
	vkEndCommandBuffer(ext_cmd);
}

bool CommandList::init() {

	ext = g->alloc<CommandListExt>();

	GraphicsExt &glext = g->getExtension();

	ext->pool = glext.pool;

	VkCommandBufferAllocateInfo allocInfo;
	memset(&allocInfo, 0, sizeof(allocInfo));

	ext->cmds = Array<VkCommandBuffer>( g->getBuffering());

	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = (u32) ext->cmds.size();
	allocInfo.commandPool = glext.pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkCheck<0x2, CommandListExt>(vkAllocateCommandBuffers(glext.device, &allocInfo, ext->cmds.begin()), "Couldn't allocate command list");

	for (u32 i = 0; i < allocInfo.commandBufferCount; ++i)
		vkName(glext, ext->cmds[i], VK_OBJECT_TYPE_COMMAND_BUFFER, getName() + " #" + i);

	return true;
}

void CommandList::bind(Pipeline *pipeline) {

	if (pipeline->getMeshBuffer() != boundMB) {

		boundMB = pipeline->getMeshBuffer();

		if (boundMB != nullptr)
			bind(boundMB);
	}

	if (pipeline->getPipelineType() == PipelineType::Graphics) {

		Vec2u res = pipeline->getRenderTarget()->getSize();

		VkViewport viewport = { 0, 0, (f32) res.x, (f32) res.y, 0, 1 };
		vkCmdSetViewport(ext_cmd, 0, 1, &viewport);

		VkRect2D scissor = { { 0, 0 }, { res.x, res.y } };
		vkCmdSetScissor(ext_cmd, 0, 1, &scissor);

	}

	VkPipelineBindPoint pipelinePoint = PipelineTypeExt(pipeline->getPipelineType().getName()).getValue();
	vkCmdBindPipeline(ext_cmd, pipelinePoint, pipeline->getExtension().obj);

	pipeline->getData()->update();

	vkCmdBindDescriptorSets(ext_cmd, pipelinePoint, pipeline->getData()->getExtension().layout, 0, 1, pipeline->getData()->getExtension().descriptorSet.begin() + g->getExtension().frameId, 0, nullptr);

}

bool CommandList::bind(Array<GPUBuffer*> vbos, GPUBuffer *ibo) {

	Array<VkBuffer> vkBuffer(vbos.size());

	u32 i = 0;

	for (GPUBuffer *b : vbos)
		if (b->getType() != GPUBufferType::VBO)
			return Log::throwError<CommandListExt, 0x0>("CommandList::bind requires VBOs as first argument");
		else
			vkBuffer[i++] = b->getExtension().resource[0];

	VkDeviceSize zero = 0;

	if (vbos.size() != 0)
		vkCmdBindVertexBuffers(ext_cmd, 0, (u32) vkBuffer.size(), vkBuffer.begin(), &zero);

	if (ibo != nullptr) {

		if (ibo->getType() != GPUBufferType::IBO)
			return Log::throwError<CommandListExt, 0x1>("CommandList::bind requires a valid IBO as second argument");

		vkCmdBindIndexBuffer(ext_cmd, ibo->getExtension().resource[0], 0, VkIndexType::VK_INDEX_TYPE_UINT32);

	}

	return true;
}

void CommandList::draw(DrawList *drawList) {

	constexpr u32 arraysCmd = (u32) sizeof(VkDrawIndirectCommand), indexedCmd = (u32) sizeof(VkDrawIndexedIndirectCommand);

	const DrawListInfo &drawListInfo = drawList->getInfo();
	const MeshBufferInfo &meshBufferInfo = drawListInfo.meshBuffer->getInfo();

	VkBuffer &resource = drawListInfo.drawBuffer->getExtension().resource[g->getExtension().frameId];

	if (g->getExtension().pfeatures.multiDrawIndirect) {

		if (meshBufferInfo.maxIndices == 0)
			vkCmdDrawIndirect(ext_cmd, resource, 0, drawList->getBatches(), arraysCmd);
		else
			vkCmdDrawIndexedIndirect(ext_cmd, resource, 0, drawList->getBatches(), indexedCmd);

	} else {

		for (u32 i = 0; i < drawList->getBatches(); ++i)
			if (meshBufferInfo.maxIndices == 0)
				vkCmdDrawIndirect(ext_cmd, resource, arraysCmd * i, 1, arraysCmd);
			else
				vkCmdDrawIndexedIndirect(ext_cmd, resource, indexedCmd * i, 1, indexedCmd);

	}

}

void CommandList::dispatch(ComputeList *computeList) {
	for(u32 i = 0; i < computeList->getDispatches(); ++i)
		vkCmdDispatchIndirect(ext_cmd, 
			computeList->getDispatchBuffer()->getExtension().resource[g->getExtension().frameId], 
			i * sizeof(VkDispatchIndirectCommand)
		);
}