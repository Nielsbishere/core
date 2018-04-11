#ifdef __VULKAN__

#include "graphics/gl/vulkan.h"
#include "graphics/texture.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

bool Texture::init(Graphics *g, bool isOwned) {

	this->g = g;
	this->isOwned = isOwned;

	VkTexture &textureData = *(VkTexture*) platformData;
	VkGraphics &graphics = *(VkGraphics*) g->getPlatformData();

	bool useStencil = false, useDepth = false;

	if (this->format == TextureFormat::Depth) {

		useDepth = true;

		std::vector<VkTextureFormat> priorities = { VkTextureFormat::D32S8, VkTextureFormat::D24S8, VkTextureFormat::D32, VkTextureFormat::D16 };

		for (VkTextureFormat &f : priorities) {
			VkFormatProperties fprop;
			vkGetPhysicalDeviceFormatProperties(graphics.pdevice, (VkFormat)f.getValue().value, &fprop);

			if (fprop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				this->format = f.getName();
				break;
			}
		}

		if (this->format == TextureFormat::Depth)
			return Log::throwError<Texture, 0x0>("Couldn't get depth texture; no optimal format available");

		useStencil = this->format.getIndex() > TextureFormat::D32;
	}

	VkTextureFormat format = this->format.getName();
	VkFormat format_inter = (VkFormat) format.getValue().value;

	VkTextureUsage usage = this->usage.getName();
	VkImageLayout usage_inter = (VkImageLayout) usage.getValue().value;

	if(isOwned){
	
		//Create image

		VkImageCreateInfo imageInfo;
		memset(&imageInfo, 0, sizeof(imageInfo));

		bool useDepth = this->format.getValue() >= TextureFormat::D16 && this->format.getValue() <= TextureFormat::D32S8;

		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format_inter;
		imageInfo.extent = { size.x, size.y, 1 };
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.usage = useDepth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		vkCheck<0x1, Texture>(vkCreateImage(graphics.device, &imageInfo, allocator, &textureData.image), "Couldn't create image");

		//Allocate memory

		VkMemoryAllocateInfo memoryInfo;
		memset(&memoryInfo, 0, sizeof(memoryInfo));

		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(graphics.device, textureData.image, &requirements);

		VkMemoryPropertyFlagBits required = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		//Check if it has device space left
		uint32_t memoryIndex = u32_MAX;

		for (uint32_t i = 0; i < graphics.pmemory.memoryTypeCount; ++i)
			if (requirements.memoryTypeBits & (1 << i) && graphics.pmemory.memoryTypes[i].propertyFlags & required) {
				memoryIndex = i;
				break;
			}

		if (memoryIndex == u32_MAX)
			Log::throwError<Texture, 0x2>("Couldn't find a valid memory type for an image");

		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryIndex;

		vkCheck<0x3, Texture>(vkAllocateMemory(graphics.device, &memoryInfo, allocator, &textureData.memory), "Couldn't allocate image memory");
		vkCheck<0x4, Texture>(vkBindImageMemory(graphics.device, textureData.image, textureData.memory, 0), "Couldn't bind image memory");

	}
	
	//Create image view

	VkImageViewCreateInfo viewInfo;
	memset(&viewInfo, 0, sizeof(viewInfo));

	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureData.image;
	viewInfo.format = format_inter;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.subresourceRange.aspectMask = useDepth ? (useStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0) | VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.layerCount = 1;

	vkCheck<0x4, Texture>(vkCreateImageView(graphics.device, &viewInfo, allocator, &textureData.view), "Couldn't create image view");

	Log::println(String("Successfully created a VkTexture with format ") + this->format.getName() + " and size " + size);
	return true;
}

Texture::~Texture() {

	if (g != nullptr) {

		VkTexture &textureData = *(VkTexture*)platformData;
		VkGraphics &graphics = *(VkGraphics*)g->getPlatformData();

		vkDestroyImageView(graphics.device, textureData.view, allocator);
		
		if(isOwned){
			vkFreeMemory(graphics.device, textureData.memory, allocator);
			vkDestroyImage(graphics.device, textureData.image, allocator);
		}

	}
}

#endif