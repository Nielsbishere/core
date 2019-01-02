#ifdef __VULKAN__

#include "graphics/vulkan.h"
#include "graphics/graphics.h"
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/texture/versionedtexture.h"
using namespace oi::gc;
using namespace oi;

void RenderTarget::destroyData() {

	GraphicsExt &gext = g->getExtension();

	if(ext->renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(gext.device, ext->renderPass, vkAllocator);

	for (VkFramebuffer fb : ext->frameBuffer)
		vkDestroyFramebuffer(gext.device, fb, vkAllocator);

	g->dealloc<RenderTarget>(ext);
}

bool RenderTarget::resize(Vec2u size) {

	if (size == Vec2u())
		return false;

	info.res = size;

	GraphicsExt &gext = g->getExtension();

	u32 buffering = g->getBuffering();

	Texture *depth = getDepth();
	bool depthTarget = depth != nullptr;

	u32 ctargets = info.targets;
	u32 targets = ctargets + depthTarget;

	//Remove old frame buffers

	for (VkFramebuffer &fb : ext->frameBuffer)
		vkDestroyFramebuffer(gext.device, fb, vkAllocator);

	if (depth != nullptr)
		depth->resize(size);

	for (VersionedTexture *texture : info.textures)
		texture->resize(size);

	if (isComputeTarget())
		return true;

	//Create framebuffers

	ext->frameBuffer = std::vector<VkFramebuffer>(buffering);

	std::vector<VkImageView> fbAttachment(targets);

	for (u32 i = 0; i < buffering; ++i) {

		VkFramebufferCreateInfo fbInfo;
		memset(&fbInfo, 0, sizeof(fbInfo));

		if (depthTarget)
			fbAttachment[ctargets] = depth->getExtension().view;

		for (u32 j = 0; j < ctargets; ++j) {
			Texture *t = getTarget(j)->getVersion(i);
			TextureExt &tex = t->getExtension();
			fbAttachment[j] = tex.view;
		}

		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = ext->renderPass;
		fbInfo.width = info.res.x;
		fbInfo.height = info.res.y;
		fbInfo.layers = 1;
		fbInfo.attachmentCount = targets;
		fbInfo.pAttachments = fbAttachment.data();

		vkCheck<0x1, RenderTargetExt>(vkCreateFramebuffer(gext.device, &fbInfo, vkAllocator, ext->frameBuffer.data() + i), "Couldn't create framebuffers for render target");
		vkName(gext, ext->frameBuffer[i], VK_OBJECT_TYPE_FRAMEBUFFER, getName() + " framebuffer " + i);

	}

	return true;
}

RenderTargetExt &RenderTarget::getExtension() { return *ext; }

bool RenderTarget::initData() {

	GraphicsExt &gext = g->getExtension();

	g->alloc<RenderTarget>(ext);

	if (!info.isComputeTarget) {

		//Set up attachments

		Texture *depth = getDepth();
		bool depthTarget = depth != nullptr;

		u32 ctargets = info.targets;
		u32 targets = ctargets + depthTarget;

		std::vector<VkAttachmentDescription> attachments(targets);

		for (u32 i = 0; i < targets; ++i) {

			VkAttachmentDescription &desc = attachments[i];
			memset(&desc, 0, sizeof(desc));

			bool isDepth = depthTarget && i == ctargets;

			TextureFormat format = isDepth ? getDepth()->getFormat() : getTarget(i)->getFormat();

			VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			if (!(isDepth || getTarget(i)->isOwned()))
				finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;									//Back buffer

			desc.format = (VkFormat)TextureFormatExt(format.getName()).getValue().value;
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			desc.finalLayout = finalLayout;

			if (isDepth) {
				desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			}
		}

		//Set up subpasses

		VkSubpassDescription subpass;
		memset(&subpass, 0, sizeof(subpass));

		std::vector<VkAttachmentReference> colorAttachment(ctargets);

		for (u32 i = 0; i < ctargets; ++i)
			colorAttachment[i] = { i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = colorAttachment.data();
		subpass.colorAttachmentCount = ctargets;

		VkAttachmentReference depthRef;

		if (depth == nullptr) subpass.pDepthStencilAttachment = nullptr;
		else {
			depthRef = { ctargets, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			subpass.pDepthStencilAttachment = &depthRef;
		}

		//Create render pass

		VkRenderPassCreateInfo passInfo;
		memset(&passInfo, 0, sizeof(passInfo));

		passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		passInfo.pAttachments = attachments.data();
		passInfo.attachmentCount = targets;
		passInfo.pSubpasses = &subpass;
		passInfo.subpassCount = 1;

		vkCheck<0x0, RenderTargetExt>(vkCreateRenderPass(gext.device, &passInfo, vkAllocator, &ext->renderPass), "Couldn't create render pass for render target");
		vkName(gext, ext->renderPass, VK_OBJECT_TYPE_RENDER_PASS, getName());

		Log::println("Successfully created render pass for render target");

		if (info.res != Vec2u() && !resize(info.res))
			return false;

	}

	Log::println("Successfully created render target");

	return true;
}

#endif