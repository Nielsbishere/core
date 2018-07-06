#ifdef __VULKAN__

#include "graphics/commandlist.h"
#include "graphics/rendertarget.h"
#include "graphics/pipeline.h"
#include "graphics/shader.h"
#include "graphics/graphics.h"
#include "graphics/gbuffer.h"
#include "graphics/camera.h"
#include "graphics/drawlist.h"
#include "graphics/meshbuffer.h"
#include "graphics/versionedtexture.h"
using namespace oi::gc;
using namespace oi;

CommandList::~CommandList() { 
	vkFreeCommandBuffers(g->getExtension().device, ext.pool, 1, &ext.cmd);
}

CommandListExt &CommandList::getExtension() { return ext; }

void CommandList::begin() {

	VkCommandBufferBeginInfo beginInfo;
	memset(&beginInfo, 0, sizeof(beginInfo));

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(ext.cmd, &beginInfo);
}

void CommandList::begin(RenderTarget *target, RenderTargetClear clear) {

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
				*(Vec4f*)cl.color.float32 = Vec4f(color);

		}
	}

	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderArea.extent = { target->getSize().x, target->getSize().y };
	beginInfo.renderPass = rtext.renderPass;
	beginInfo.framebuffer = rtext.frameBuffer[g->getExtension().current];
	beginInfo.clearValueCount = (u32)clearValue.size();
	beginInfo.pClearValues = clearValue.data();

	vkCmdBeginRenderPass(ext.cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

}

void CommandList::end(RenderTarget *target) {
	vkCmdEndRenderPass(ext.cmd);
}

void CommandList::end() {
	vkEndCommandBuffer(ext.cmd);
}

bool CommandList::init() {

	GraphicsExt &glext = g->getExtension();

	VkCommandBufferAllocateInfo allocInfo;
	memset(&allocInfo, 0, sizeof(allocInfo));

	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = 1;
	allocInfo.commandPool = glext.pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkCheck<0x0, CommandList>(vkAllocateCommandBuffers(glext.device, &allocInfo, &ext.cmd), "Couldn't allocate command list");

	return true;
}

void CommandList::bind(Pipeline *pipeline) {

	VkPipelineBindPoint pipelinePoint = pipeline->getInfo().shader->isCompute() ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkCmdBindPipeline(ext.cmd, pipelinePoint, pipeline->getExtension());

	pipeline->getInfo().shader->update();

	vkCmdBindDescriptorSets(ext.cmd, pipelinePoint, pipeline->getInfo().shader->getExtension().layout, 0, 1, pipeline->getInfo().shader->getExtension().descriptorSet.data() + g->getExtension().current, 0, nullptr);

	if (pipeline->getInfo().camera != nullptr) {

		pipeline->getInfo().camera->bind(pipeline->getInfo().renderTarget->getSize());
		CameraStruct cs = pipeline->getInfo().camera->getBound();

		ShaderBuffer *sb = pipeline->getInfo().shader->get<ShaderBuffer>("Camera");

		if (sb == nullptr)
			Log::throwError<CommandList, 0x5>("Pipeline has camera bound, but there is no Camera buffer.");

		sb->set(Buffer::construct((u8*)&cs, (u32) sizeof(cs)));

	}

}

void CommandList::draw(u32 vertices, u32 instances, u32 startVertex, u32 startInstance) {
	vkCmdDraw(ext.cmd, vertices, instances, startVertex, startInstance);
}

void CommandList::drawIndexed(u32 indices, u32 instances, u32 startIndex, u32 startVertex, u32 startInstance) {
	vkCmdDrawIndexed(ext.cmd, indices, instances, startIndex, startVertex, startInstance);
}

bool CommandList::bind(std::vector<GBuffer*> vbos, GBuffer *ibo) {

	std::vector<VkBuffer> vkBuffer(vbos.size());

	u32 i = 0;

	for (GBuffer *b : vbos)
		if (b->getType() != GBufferType::VBO)
			return Log::throwError<CommandList, 0x1>("CommandList::bind requires VBOs as first argument");
		else
			vkBuffer[i++] = b->getExtension().resource;

	VkDeviceSize zero = 0;

	if (vbos.size() != 0)
		vkCmdBindVertexBuffers(ext.cmd, 0, (u32) vkBuffer.size(), vkBuffer.data(), &zero);

	if (ibo != nullptr) {

		if (ibo->getType() != GBufferType::IBO)
			return Log::throwError<CommandList, 0x2>("CommandList::bind requires a valid IBO as second argument");

		vkCmdBindIndexBuffer(ext.cmd, ibo->getExtension().resource, 0, VkIndexType::VK_INDEX_TYPE_UINT32);

	}

	return true;
}

void CommandList::flush() {

	end();

	VkSubmitInfo submitInfo;
	memset(&submitInfo, 0, sizeof(submitInfo));

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1U;
	submitInfo.pCommandBuffers = &ext.cmd;

	vkCheck<0x4, CommandList>(vkQueueSubmit(g->getExtension().queue, 1, &submitInfo, nullptr), "Couldn't submit queue");

	g->finish();
}

void CommandList::draw(DrawList *drawList) {

	bind(drawList->getInfo().meshBuffer);

	if (drawList->getInfo().meshBuffer->getInfo().maxIndices == 0)
		vkCmdDrawIndirect(ext.cmd, drawList->getInfo().drawBuffer->getExtension().resource, 0, drawList->getBatches(), (u32) sizeof(VkDrawIndirectCommand));
	else
		vkCmdDrawIndexedIndirect(ext.cmd, drawList->getInfo().drawBuffer->getExtension().resource, 0, drawList->getBatches(), (u32) sizeof(VkDrawIndexedIndirectCommand));

}

#endif