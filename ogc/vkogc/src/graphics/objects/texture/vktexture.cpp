#include "graphics/graphics.h"
#include "graphics/vulkan.h"
#include "graphics/objects/texture/texture.h"
#include "graphics/objects/gpubuffer.h"
#include "graphics/objects/texture/texturelist.h"
#include "graphics/objects/render/commandlist.h"
#include "graphics/objects/texture/vktexture.h"
#include "graphics/objects/vkgpubuffer.h"
#include "graphics/objects/render/vkcommandlist.h"
using namespace oi::gc;
using namespace oi;

TextureExt &Texture::getExtension() { return *ext; }

bool Texture::initData() {

	if(ext == nullptr)
		g->alloc<Texture>(ext);

	GraphicsExt &graphics = g->getExtension();

	if (info.format == TextureFormat::Depth || info.format == TextureFormat::Depth_stencil) {

		static const std::vector<TextureFormatExt> stencil = { TextureFormatExt::D32S8, TextureFormatExt::D24S8, TextureFormatExt::D16S8 };
		static const std::vector<TextureFormatExt> depth = { TextureFormatExt::D32, TextureFormatExt::D16 };

		static const std::vector<TextureFormatExt> &priorities = info.format == TextureFormat::Depth ? depth : stencil;

		for (const TextureFormatExt &f : priorities) {
			VkFormatProperties fprop;
			vkGetPhysicalDeviceFormatProperties(graphics.pdevice, (VkFormat) f.getValue().value, &fprop);

			if (fprop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				info.format = f.getName();
				break;
			}
		}

		if (info.format == TextureFormat::Depth || info.format == TextureFormat::Depth_stencil)
			return Log::throwError<TextureExt, 0x0>("Couldn't get depth texture; no optimal format available");

	}

	bool useDepth = Graphics::isDepthFormat(info.format), useStencil = Graphics::hasStencil(info.format);

	TextureFormatExt format = info.format.getName();
	VkFormat format_inter = (VkFormat) format.getValue().value;

	TextureUsageExt usage = info.usage.getName();

	if (info.res.x != 0 && info.res.y != 0) {

		if (owned) {

			//Create image

			VkImageCreateInfo imageInfo;
			memset(&imageInfo, 0, sizeof(imageInfo));

			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = format_inter;
			imageInfo.extent = { info.res.x, info.res.y, 1 };
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.mipLevels = info.mipLevels;
			imageInfo.arrayLayers = 1;
			imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			switch (info.usage.getValue()) {

			case TextureUsage::Image.value:
				imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				break;

			case TextureUsage::Compute_target.value:
				imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
				break;

			case TextureUsage::Render_depth.value:
				imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				break;

			default:
				imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				break;

			}

			vkCheck<0x2, TextureExt>(vkCreateImage(graphics.device, &imageInfo, vkAllocator, &ext->resource), "Couldn't create image");
			vkName(graphics, ext->resource, VK_OBJECT_TYPE_IMAGE, getName());

			graphics.alloc(*ext, getName());

		}

		//Create image view

		VkImageViewCreateInfo viewInfo;
		memset(&viewInfo, 0, sizeof(viewInfo));

		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = ext->resource;
		viewInfo.format = format_inter;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.subresourceRange.aspectMask = useDepth ? (useStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0) | VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = info.mipLevels;
		viewInfo.subresourceRange.layerCount = 1;

		vkCheck<0x3, TextureExt>(vkCreateImageView(graphics.device, &viewInfo, vkAllocator, &ext->view), "Couldn't create image view");
		vkName(graphics, ext->view, VK_OBJECT_TYPE_IMAGE_VIEW, getName() + " view");

		//Prepare texture for update

		if (info.dat.size() != 0U) {

			if (info.dat.size() != info.res.x * info.res.y * Graphics::getFormatSize(info.format))
				return Log::throwError<TextureExt, 0x1>("The buffer was of incorrect size");

			flush(Vec2u(), info.res);

		}

		Log::println(String("Successfully created a VkTexture with format ") + info.format.getName() + " and size " + info.res);
	}

	if (info.parent != nullptr) {
		info.handle = info.parent->alloc(this);
		g->use(info.parent);
	}

	return true;
}

bool Texture::getPixelsGpu(Vec2u start, Vec2u length, CopyBuffer &output) {

	if (!owned || info.usage != TextureUsage::Image)
		return Log::throwError<TextureExt, 0xF>("Couldn't get pixels; resource has to be owned by the application (render target or depth buffer isn't allowed)");

	GraphicsExt &graphics = g->getExtension();

	//Create command list

	VkCommandBufferAllocateInfo commandInfo;
	memset(&commandInfo, 0, sizeof(commandInfo));

	commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandInfo.commandPool = graphics.pool;
	commandInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandInfo.commandBufferCount = 1;

	VkCommandBuffer cmd;
	vkCheck<0x6, TextureExt>(vkAllocateCommandBuffers(graphics.device, &commandInfo, &cmd), "Couldn't allocate intermediate command list");
	vkName(graphics, cmd, VK_OBJECT_TYPE_COMMAND_BUFFER, getName() + " intermediate commands");

	//Create submit fence

	VkFenceCreateInfo fenceInfo;
	memset(&fenceInfo, 0, sizeof(fenceInfo));

	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VkFence fence;
	vkCheck<0x7, TextureExt>(vkCreateFence(graphics.device, &fenceInfo, vkAllocator, &fence), "Couldn't allocate intermediate fence");

	//Get image layout & aspect

	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	if(info.usage == TextureUsage::Render_depth)
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (Graphics::hasStencil(info.format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);

	//Create intermediate buffer

	u32 stride = (info.loadFormat.getValue() - 1) % 4 + 1;

	VkBufferCreateInfo bufferInfo;
	memset(&bufferInfo, 0, sizeof(bufferInfo));

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = length.y * length.x * stride;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferInfo.queueFamilyIndexCount = 1;
	bufferInfo.pQueueFamilyIndices = &graphics.queueFamilyIndex;

	VkBuffer dst;

	vkCheck<0xA, TextureExt>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, &dst), "Failed to create intermediate buffer");
	vkName(graphics, dst, VK_OBJECT_TYPE_BUFFER, getName() + " intermediate");

	//Allocate memory (TODO: by GraphicsExt)

	VkDeviceMemory dstMemory;

	VkMemoryAllocateInfo memoryInfo;
	memset(&memoryInfo, 0, sizeof(memoryInfo));

	VkMemoryRequirements2 requirements2;
	memset(&requirements2, 0, sizeof(requirements2));

	requirements2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR;

	VkBufferMemoryRequirementsInfo2 bufferMemoryRequirements2;
	memset(&bufferMemoryRequirements2, 0, sizeof(bufferMemoryRequirements2));

	bufferMemoryRequirements2.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR;
	bufferMemoryRequirements2.buffer = dst;

	graphics.vkGetBufferMemoryRequirements2(graphics.device, &bufferMemoryRequirements2, &requirements2);
	VkMemoryRequirements &requirements = requirements2.memoryRequirements;

	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.allocationSize = requirements.size;

	uint32_t memoryIndex = u32_MAX;

	for (uint32_t i = 0; i < graphics.pmemory.memoryTypeCount; ++i)
		if ((requirements.memoryTypeBits & (1 << i)) && (graphics.pmemory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
			memoryIndex = i;
			break;
		}

	if (memoryIndex == u32_MAX)
		Log::throwError<TextureExt, 0xB>(String("Couldn't find a valid memory type for a VkBuffer: ") + getName() + " intermediate");

	memoryInfo.memoryTypeIndex = memoryIndex;

	vkCheck<0xC, TextureExt>(vkAllocateMemory(graphics.device, &memoryInfo, vkAllocator, &dstMemory), "Couldn't allocate memory");
	vkCheck<0xD, TextureExt>(vkBindBufferMemory(graphics.device, dst, dstMemory, 0), String("Couldn't bind memory to buffer ") + getName() + " intermediate");

	//Copy to intermediate buffer

	VkBufferImageCopy region;
	memset(&region, 0, sizeof(region));

	region.imageSubresource = { aspectMask, 0, 0, 1 };
	region.imageOffset = { (i32)start.x, (i32)start.y, 0 };
	region.imageExtent = { length.x, length.y, 1 };

	vkCmdCopyImageToBuffer(cmd, ext->resource, layout, dst, 1, &region);

	//Flush command list

	VkSubmitInfo submitInfo;
	memset(&submitInfo, 0, sizeof(submitInfo));

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	vkCheck<0x8, TextureExt>(vkQueueSubmit(graphics.queue, 1, &submitInfo, fence), "Couldn't submit intermediate commands");
	vkCheck<0x9, TextureExt>(vkWaitForFences(graphics.device, 1, &fence, true, u64_MAX), "Couldn't wait for intermediate fences");

	//Destroy command buffer and fence

	vkDestroyFence(graphics.device, fence, vkAllocator);
	vkFreeCommandBuffers(graphics.device, graphics.pool, 1, &cmd);

	//Copy buffer to copy buffer
	void *memory;
	vkCheck<0xE, TextureExt>(vkMapMemory(graphics.device, dstMemory, 0, VK_WHOLE_SIZE, 0, &memory), "Couldn't map intermediate memory");
	output = CopyBuffer((u8*)memory, (u32) requirements.size);
	vkUnmapMemory(graphics.device, dstMemory);

	//Destroy buffer

	vkFreeMemory(graphics.device, dstMemory, vkAllocator);
	vkDestroyBuffer(graphics.device, dst, vkAllocator);

	return true;

}

void Texture::destroyData(bool resize) {

	if (g != nullptr && info.res.x != 0 && info.res.y != 0) {

		GraphicsExt &graphics = g->getExtension();

		vkDestroyImageView(graphics.device, ext->view, vkAllocator);
		
		if(owned)
			graphics.dealloc(*ext, getName());

	}

	if(!resize)
		g->dealloc<Texture>(ext);

}

void Texture::push() {

	if (!shouldStage())
		return;

	GraphicsExt &graphics = g->getExtension();

	//Prepare data for transfer

	Vec2u changedLength = info.changedEnd - info.changedStart;

	u32 stride = getStride();
	u32 size = changedLength.x * changedLength.y * stride;
	bool fullTexture = changedLength == info.res;

	Buffer dat = fullTexture ? info.dat : Buffer(size);

	if (!fullTexture) {

		if (changedLength.x == info.res.x)		//Copy rows (fast)
			memcpy(dat.addr(), info.dat.addr() + info.changedStart.y * changedLength.x * stride, size);
		else {

			//Copy rows (slow)
			for(u32 i = 0; i < changedLength.y; ++i)
				memcpy(
					dat.addr() + i * changedLength.x * stride,
					info.dat.addr() + ((info.changedStart.y + i) * info.res.x + info.changedStart.x) * stride,
					changedLength.x * stride
				);

		}

	}

	//Construct staging buffer with data

	GPUBufferExt gbext;
	gbext.resource.resize(1);

	VkBufferCreateInfo stagingInfo;
	memset(&stagingInfo, 0, sizeof(stagingInfo));

	stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingInfo.size = size;
	stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	stagingInfo.queueFamilyIndexCount = 1;
	stagingInfo.pQueueFamilyIndices = &graphics.queueFamilyIndex;

	vkCheck<0x4, TextureExt>(vkCreateBuffer(graphics.device, &stagingInfo, vkAllocator, gbext.resource.data()), "Couldn't send texture data to GPU");
	vkName(graphics, gbext.resource[0], VK_OBJECT_TYPE_IMAGE, getName() + " staging buffer");

	graphics.alloc(gbext, GPUBufferType::SSBO /* unused */, getName() + " staging buffer", true);

	//Copy data to staging buffer

	auto balloc = gbext.allocations[0];
	memcpy(balloc.mappedMemory.addr(), dat.addr(), size);

	//Copy data to cmd list

	CommandListExt &cmd = graphics.stagingCmdList->getExtension();

	VkFilter filter = info.mipFilter != TextureMipFilter::Nearest ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

	//Transition to write

	VkImageMemoryBarrier barrier;
	memset(&barrier, 0, sizeof(barrier));

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.image = ext->resource;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.levelCount = info.mipLevels;
	barrier.subresourceRange.layerCount = 1;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	vkCmdPipelineBarrier(cmd.cmds[0], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	//Copy it into the texture

	VkBufferImageCopy region;
	memset(&region, 0, sizeof(region));

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1U;
	region.imageOffset = { (i32) info.changedStart.x, (i32) info.changedStart.y, 0 };
	region.imageExtent = { changedLength.x, changedLength.y, 1 };

	vkCmdCopyBufferToImage(cmd.cmds[0], gbext.resource[0], ext->resource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	//Generate mipmaps

	memset(&barrier, 0, sizeof(barrier));

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = ext->resource;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	u32 mipWidth = info.res.x, mipHeight = info.res.y;

	for (u32 i = 1; i < info.mipLevels; ++i) {

		//Transition mipmap source from DST_OPTIMAL to SRC_OPTIMAL

		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(cmd.cmds[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		//Create mipmap from mip i - 1 into i

		VkImageBlit blit;
		memset(&blit, 0, sizeof(blit));

		blit.srcOffsets[1] = { (i32)mipWidth, (i32)mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1U;
		blit.srcSubresource.layerCount = 1U;

		blit.dstOffsets[1] = { (i32)mipWidth >> 1, (i32)mipHeight >> 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.layerCount = 1U;

		vkCmdBlitImage(cmd.cmds[0], ext->resource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ext->resource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, filter);

		if (mipWidth > 1) mipWidth >>= 1U;
		if (mipHeight > 1) mipHeight >>= 1U;

	}

	//Last mip is in DST_OPTIMAL and ones before that in SRC_OPTIMAL
	//So transition them back to SHADER_READ_OPTIMAL
	//Two barriers; 0 to mipLevels - 1 from SRC to SHADER_READ and mipLevels - 1 from DST to SHADER_READ

	memset(&barrier, 0, sizeof(barrier));

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = ext->resource;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.levelCount = info.mipLevels - 1U;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkImageMemoryBarrier barrier0 = barrier;
	barrier0.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier0.subresourceRange.levelCount = 1U;
	barrier0.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier0.subresourceRange.baseMipLevel = info.mipLevels - 1U;

	VkImageMemoryBarrier barriers[] = { barrier, barrier0 };

	vkCmdPipelineBarrier(cmd.cmds[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, info.mipLevels == 1 ? 1U : 2U, info.mipLevels == 1 ? barriers + 1 : barriers);

	//Clean up staging buffer and free memory

	graphics.stagingBuffers[graphics.frameId][getName() + " staging buffer"] = gbext;

	if (!fullTexture)
		dat.deconstruct();

	info.changedStart = Vec2u(u32_MAX, u32_MAX);
	info.changedEnd = Vec2u();

}