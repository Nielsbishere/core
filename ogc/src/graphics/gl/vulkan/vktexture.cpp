#ifdef __VULKAN__

#include "graphics/gl/vulkan.h"
#include "graphics/texture.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

bool Texture::init(bool isOwned) {

	this->owned = isOwned;

	VkGraphics &graphics = g->getExtension();

	bool useStencil = false, useDepth = false;

	if (info.format == TextureFormat::Depth) {

		useDepth = true;

		std::vector<VkTextureFormat> priorities = { VkTextureFormat::D32S8, VkTextureFormat::D24S8, VkTextureFormat::D32, VkTextureFormat::D16 };

		for (VkTextureFormat &f : priorities) {
			VkFormatProperties fprop;
			vkGetPhysicalDeviceFormatProperties(graphics.pdevice, (VkFormat) f.getValue().value, &fprop);

			if (fprop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				info.format = f.getName();
				break;
			}
		}

		if (info.format == TextureFormat::Depth)
			return Log::throwError<Texture, 0x0>("Couldn't get depth texture; no optimal format available");

		useStencil = info.format.getIndex() > TextureFormat::D32;
	}

	VkTextureFormat format = info.format.getName();
	VkFormat format_inter = (VkFormat) format.getValue().value;

	VkTextureUsage usage = info.usage.getName();
	VkImageLayout usage_inter = (VkImageLayout) usage.getValue().value;

	if(isOwned){
	
		//Create image

		VkImageCreateInfo imageInfo;
		memset(&imageInfo, 0, sizeof(imageInfo));

		bool useDepth = info.format.getValue() >= TextureFormat::D16 && info.format.getValue() <= TextureFormat::D32S8;

		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format_inter;
		imageInfo.extent = { info.res.x, info.res.y, 1 };
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.usage = useDepth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		vkCheck<0x1, Texture>(vkCreateImage(graphics.device, &imageInfo, allocator, &ext.resource), "Couldn't create image");

		//Allocate memory
		vkAllocate(Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}
	
	//Create image view

	VkImageViewCreateInfo viewInfo;
	memset(&viewInfo, 0, sizeof(viewInfo));

	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = ext.resource;
	viewInfo.format = format_inter;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.subresourceRange.aspectMask = useDepth ? (useStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0) | VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.layerCount = 1;

	vkCheck<0x2, Texture>(vkCreateImageView(graphics.device, &viewInfo, allocator, &ext.view), "Couldn't create image view");

	Log::println(String("Successfully created a VkTexture with format ") + info.format.getName() + " and size " + info.res);
	return true;
}

Texture::~Texture() {

	if (g != nullptr) {

		VkGraphics &graphics = g->getExtension();

		vkDestroyImageView(graphics.device, ext.view, allocator);
		
		if(owned){
			vkFreeMemory(graphics.device, ext.memory, allocator);
			vkDestroyImage(graphics.device, ext.resource, allocator);
		}

	}
}

#endif