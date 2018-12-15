#ifdef __VULKAN__

#include "graphics/graphics.h"
#include "graphics/objects/gbuffer.h"
#include "graphics/objects/render/commandlist.h"
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/render/drawlist.h"
#include "graphics/objects/shader/computelist.h"
#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/model/mesh.h"
using namespace oi::gc;
using namespace oi;

CommandList::~CommandList() {
	vkFreeCommandBuffers(g->getExtension().device, ext.pool, (u32) ext.cmds.size(), ext.cmds.data());
}

CommandListExt &CommandList::getExtension() { return ext; }

VkCommandBuffer &VkCommandList::cmd(VkGraphics &g) { return cmds[g.current % cmds.size()]; }
#define ext_cmd ext.cmd(g->getExtension())

void CommandList::begin() {

	VkCommandBufferBeginInfo beginInfo;
	memset(&beginInfo, 0, sizeof(beginInfo));

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(ext_cmd, &beginInfo);
}

void CommandList::begin(RenderTarget *target, RenderTargetClear clear) {

	if (target->isComputeTarget()) {

		u32 frame = g->getExtension().current;

		VkImageMemoryBarrier imageBarrier;
		memset(&imageBarrier, 0, sizeof(imageBarrier));

		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		VkImageMemoryBarrier *barriers = new VkImageMemoryBarrier[target->getTargets()];

		for (u32 i = 0; i < target->getTargets(); ++i)
			(barriers[i] = imageBarrier).image = target->getTarget(i)->getVersion(frame)->getExtension().resource;

		vkCmdPipelineBarrier(ext_cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, target->getTargets(), barriers);
		delete[] barriers;

		return;
	}

	RenderTargetExt &rtext = target->getExtension();

	VkRenderPassBeginInfo beginInfo;
	memset(&beginInfo, 0, sizeof(beginInfo));

	std::vector<VkClearValue> clearValue(target->getTargets() + 1);

	for (u32 i = 0; i < target->getTargets() + 1; ++i) {

		VkClearValue &cl = clearValue[i];
		TextureFormat format = i == 0 ? target->getDepth()->getFormat() : target->getTarget(i - 1)->getFormat();

		if (i == 0) {
			cl.depthStencil.depth = clear.depthClear;
			cl.depthStencil.stencil = clear.stencilClear;
		}
		else {

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
	beginInfo.framebuffer = rtext.frameBuffer[g->getExtension().current];
	beginInfo.clearValueCount = (u32)clearValue.size();
	beginInfo.pClearValues = clearValue.data();

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

		for (u32 i = 0; i < target->getTargets(); ++i)
			(barriers[i] = imageBarrier).image = target->getTarget(i)->getVersion(g->getExtension().current)->getExtension().resource;

		vkCmdPipelineBarrier(ext_cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, target->getTargets(), barriers);
		delete[] barriers;
	}
}

void CommandList::end() {
	vkEndCommandBuffer(ext_cmd);
}

bool CommandList::init() {

	GraphicsExt &glext = g->getExtension();

	ext.pool = glext.pool;

	VkCommandBufferAllocateInfo allocInfo;
	memset(&allocInfo, 0, sizeof(allocInfo));

	ext.cmds.resize( g->getBuffering());

	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = (u32) ext.cmds.size();
	allocInfo.commandPool = glext.pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkCheck<0x2, VkCommandList>(vkAllocateCommandBuffers(glext.device, &allocInfo, ext.cmds.data()), "Couldn't allocate command list");

	for (u32 i = 0; i < allocInfo.commandBufferCount; ++i)
		vkName(glext, ext.cmds[i], VK_OBJECT_TYPE_COMMAND_BUFFER, getName() + " #" + i);

	return true;
}

void CommandList::bind(Pipeline *pipeline) {

	if (pipeline->getInfo().meshBuffer != boundMB) {

		boundMB = pipeline->getInfo().meshBuffer;

		if (boundMB != nullptr)
			bind(boundMB);
	}

	VkPipelineBindPoint pipelinePoint = pipeline->getInfo().shader->isCompute() ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkCmdBindPipeline(ext_cmd, pipelinePoint, pipeline->getExtension());

	pipeline->getInfo().shader->update();

	vkCmdBindDescriptorSets(ext_cmd, pipelinePoint, pipeline->getInfo().shader->getExtension().layout, 0, 1, pipeline->getInfo().shader->getExtension().descriptorSet.data() + g->getExtension().current, 0, nullptr);

}

bool CommandList::bind(std::vector<GBuffer*> vbos, GBuffer *ibo) {

	std::vector<VkBuffer> vkBuffer(vbos.size());

	u32 i = 0;

	for (GBuffer *b : vbos)
		if (b->getType() != GBufferType::VBO)
			return Log::throwError<VkCommandList, 0x0>("CommandList::bind requires VBOs as first argument");
		else
			vkBuffer[i++] = b->getExtension().resource[0];

	VkDeviceSize zero = 0;

	if (vbos.size() != 0)
		vkCmdBindVertexBuffers(ext_cmd, 0, (u32) vkBuffer.size(), vkBuffer.data(), &zero);

	if (ibo != nullptr) {

		if (ibo->getType() != GBufferType::IBO)
			return Log::throwError<VkCommandList, 0x1>("CommandList::bind requires a valid IBO as second argument");

		vkCmdBindIndexBuffer(ext_cmd, ibo->getExtension().resource[0], 0, VkIndexType::VK_INDEX_TYPE_UINT32);

	}

	return true;
}

void CommandList::draw(DrawList *drawList) {

	constexpr u32 arraysCmd = (u32) sizeof(VkDrawIndirectCommand), indexedCmd = (u32) sizeof(VkDrawIndexedIndirectCommand);

	const DrawListInfo &drawListInfo = drawList->getInfo();
	const MeshBufferInfo &meshBufferInfo = drawListInfo.meshBuffer->getInfo();

	VkBuffer &resource = drawListInfo.drawBuffer->getExtension().resource[g->getExtension().current];

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
	vkCmdDispatchIndirect(ext_cmd, computeList->getDispatchBuffer()->getExtension().resource[g->getExtension().current], 0);
}

#endif