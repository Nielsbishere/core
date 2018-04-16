#ifdef __VULKAN__

#include "graphics/commandlist.h"
#include "graphics/rendertarget.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

CommandList::~CommandList() { 
	gl->cleanCommandList(this);
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

	std::vector<VkClearValue> clearValue(target->getTargets());

	for (u32 i = 0; i < target->getTargets(); ++i) {

		VkClearValue &cl = clearValue[i];
		Texture *targ = target->getTarget(i, 0);
		TextureFormat format = targ->getFormat();
		
		if (gl->isDepthFormat(format)) {
			cl.depthStencil.depth = clear.depthClear;
			cl.depthStencil.stencil = clear.stencilClear;
		} else {

			Vec4d color = gl->convertColor(clear.colorClear, format);

			TextureFormatStorage storedFormat = gl->getFormatStorage(format);

			if (storedFormat == TextureFormatStorage::INT)
				*(Vec4i*) cl.color.int32 = Vec4i(color);
			else if (storedFormat == TextureFormatStorage::UINT)
				*(Vec4u*)cl.color.uint32 = Vec4u(color);
			else
				*(Vec4f*) cl.color.float32 = Vec4f(color);

		}
	}

	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderArea.extent = { target->getSize().x, target->getSize().y };
	beginInfo.renderPass = rtext.renderPass;
	beginInfo.framebuffer = rtext.frameBuffer[gl->getExtension().current];
	beginInfo.clearValueCount = (u32) clearValue.size();
	beginInfo.pClearValues = clearValue.data();

	vkCmdBeginRenderPass(ext.cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandList::end(RenderTarget *target) {
	vkCmdEndRenderPass(ext.cmd);
}

void CommandList::end() {
	vkEndCommandBuffer(ext.cmd);
}

bool CommandList::init(Graphics *gl) {

	this->gl = gl;

	GraphicsExt &glext = gl->getExtension();

	VkCommandBufferAllocateInfo allocInfo;
	memset(&allocInfo, 0, sizeof(allocInfo));

	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = 1;
	allocInfo.commandPool = glext.pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkCheck<0x0, CommandList>(vkAllocateCommandBuffers(glext.device, &allocInfo, &ext.cmd), "Couldn't allocate command list");

	return true;
}


#endif