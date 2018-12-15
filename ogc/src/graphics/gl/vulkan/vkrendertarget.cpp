#ifdef __VULKAN__

#include "graphics/gl/vulkan.h"
#include "graphics/graphics.h"
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/texture/versionedtexture.h"
using namespace oi::gc;
using namespace oi;

RenderTarget::~RenderTarget() {

	VkGraphics &gext = g->getExtension();

	if(ext.renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(gext.device, ext.renderPass, vkAllocator);

	for (VkFramebuffer fb : ext.frameBuffer)
		vkDestroyFramebuffer(gext.device, fb, vkAllocator);

	for (VersionedTexture *t : info.textures)
		g->destroy(t);

	g->destroy(info.depth);
}

bool RenderTarget::initData() {

	u32 buffering = g->getBuffering();

	VkGraphics &gext = g->getExtension();

	if (!info.isComputeTarget) {

		//Set up attachments

		std::vector<VkAttachmentDescription> attachments(info.targets + 1);

		for (u32 i = 0; i < (u32)attachments.size(); ++i) {

			VkAttachmentDescription &desc = attachments[i];
			memset(&desc, 0, sizeof(desc));

			TextureFormat format = i == 0 ? getDepth()->getFormat() : getTarget(i - 1)->getFormat();
			bool isDepth = i == 0;

			VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			if (!(isDepth || getTarget(i - 1)->isOwned()))
				finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;									//Back buffer

			desc.format = (VkFormat)VkTextureFormat(format.getName()).getValue().value;
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

		std::vector<VkAttachmentReference> colorAttachment(info.targets);

		for (u32 i = 0; i < info.targets; ++i)
			colorAttachment[i] = { i + 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = colorAttachment.data();
		subpass.colorAttachmentCount = (u32)colorAttachment.size();

		Texture *depth;
		VkAttachmentReference depthRef;

		if ((depth = getDepth()) == nullptr) subpass.pDepthStencilAttachment = nullptr;
		else {
			depthRef = { 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			subpass.pDepthStencilAttachment = &depthRef;
		}

		//Create render pass

		VkRenderPassCreateInfo passInfo;
		memset(&passInfo, 0, sizeof(passInfo));

		passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		passInfo.pAttachments = attachments.data();
		passInfo.attachmentCount = (u32)attachments.size();
		passInfo.pSubpasses = &subpass;
		passInfo.subpassCount = 1;

		vkCheck<0x0, VkRenderTarget>(vkCreateRenderPass(gext.device, &passInfo, vkAllocator, &ext.renderPass), "Couldn't create render pass for render target");
		vkName(gext, ext.renderPass, VK_OBJECT_TYPE_RENDER_PASS, getName());

		Log::println("Successfully created render pass for render target");

		//Create framebuffers

		ext.frameBuffer.resize(buffering);

		for (u32 i = 0; i < buffering; ++i) {

			VkFramebufferCreateInfo fbInfo;
			memset(&fbInfo, 0, sizeof(fbInfo));

			std::vector<VkImageView> fbAttachment(info.targets + 1);
			fbAttachment[0] = getDepth()->getExtension().view;

			for (u32 j = 0; j < info.targets; ++j) {
				Texture *t = getTarget(j)->getVersion(i);
				VkTexture &tex = t->getExtension();
				fbAttachment[j + 1] = tex.view;
			}

			fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbInfo.renderPass = ext.renderPass;
			fbInfo.width = info.res.x;
			fbInfo.height = info.res.y;
			fbInfo.layers = 1;
			fbInfo.attachmentCount = info.targets + 1;
			fbInfo.pAttachments = fbAttachment.data();

			vkCheck<0x1, VkRenderTarget>(vkCreateFramebuffer(gext.device, &fbInfo, vkAllocator, ext.frameBuffer.data() + i), "Couldn't create framebuffers for render target");
			vkName(gext, ext.frameBuffer[i], VK_OBJECT_TYPE_FRAMEBUFFER, getName() + " framebuffer " + i);

		}

	}

	Log::println("Successfully created framebuffers for render target");

	return true;
}

#endif