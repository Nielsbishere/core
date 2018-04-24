#ifdef __VULKAN__

#include "graphics/rendertarget.h"
#include "graphics/gl/vulkan.h"
#include "graphics/graphics.h"
#include "graphics/texture.h"
using namespace oi::gc;
using namespace oi;

RenderTarget::~RenderTarget() {

	VkGraphics &gext = g->getExtension();

	vkDestroyRenderPass(gext.device, ext.renderPass, allocator);

	for (VkFramebuffer fb : ext.frameBuffer)
		vkDestroyFramebuffer(gext.device, fb, allocator);

	for (Texture *t : textures)
		g->destroy(t);

	g->destroy(depth);
}

bool RenderTarget::init() {

	VkGraphics &gext = g->getExtension();

	//Set up attachments

	std::vector<VkAttachmentDescription> attachments(info.targets);

	for (u32 i = 0; i < (u32)attachments.size(); ++i) {

		VkAttachmentDescription &desc = attachments[i];
		memset(&desc, 0, sizeof(desc));

		Texture *target = getTarget(i, 0);
		TextureFormat format = target->getFormat();
		bool isDepth = format.getValue() >= TextureFormat::D16 && format.getValue() < TextureFormat::Depth;	//Don't include 'Depth' as that is only for auto type detection

		VkImageLayout layout;

		if (isDepth) layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;							//Depth buffer
		else if (target->isOwned()) layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;					//Regular texture
		else layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;													//Back buffer

		desc.format = (VkFormat) VkTextureFormat(format.getName()).getValue().value;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		desc.finalLayout = layout;

		if (isDepth) {
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		}
	}

	//Set up subpasses TODO: Abstract this?

	VkSubpassDescription subpass;
	memset(&subpass, 0, sizeof(subpass));

	std::vector<VkAttachmentReference> colorAttachment(info.targets - 1);
	
	for (u32 i = 1; i < info.targets; ++i)
		colorAttachment[i - 1] = { i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorAttachment.data();
	subpass.colorAttachmentCount = (u32) colorAttachment.size();

	Texture *depth;
	VkAttachmentReference depthRef;

	if ((depth = getTarget(0, 0)) == nullptr) subpass.pDepthStencilAttachment = nullptr;
	else {
		depthRef = { 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		subpass.pDepthStencilAttachment = &depthRef;
	}

	//Create render pass

	VkRenderPassCreateInfo passInfo;
	memset(&passInfo, 0, sizeof(passInfo));

	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	passInfo.pAttachments = attachments.data();
	passInfo.attachmentCount = (u32) attachments.size();
	passInfo.pSubpasses = &subpass;
	passInfo.subpassCount = 1;

	vkCheck<0x0, RenderTarget>(vkCreateRenderPass(gext.device, &passInfo, allocator, &ext.renderPass), "Couldn't create render pass for render target");

	Log::println("Successfully created render pass for render target");

	//Create framebuffers

	ext.frameBuffer.resize(info.buffering);

	for (u32 i = 0; i < info.buffering; ++i) {

		VkFramebuffer *fb = ext.frameBuffer.data() + i;

		VkFramebufferCreateInfo fbInfo;
		memset(&fbInfo, 0, sizeof(fbInfo));

		std::vector<VkImageView> fbAttachment(info.targets);
		for (u32 j = 0; j < info.targets; ++j) {
			Texture *t = getTarget(j, i);
			VkTexture &tex = t->getExtension();
			fbAttachment[j] = tex.view;
		}

		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = ext.renderPass;
		fbInfo.width = info.res.x;
		fbInfo.height = info.res.y;
		fbInfo.layers = 1;
		fbInfo.attachmentCount = info.targets;
		fbInfo.pAttachments = fbAttachment.data();

		vkCheck<0x1, RenderTarget>(vkCreateFramebuffer(gext.device, &fbInfo, allocator, ext.frameBuffer.data() + i), "Couldn't create framebuffers for render target");
	}

	for(Texture *t : textures)
		g->use(t);

	g->use(depth);

	Log::println("Successfully created framebuffers for render target");

	return true;
}

#endif