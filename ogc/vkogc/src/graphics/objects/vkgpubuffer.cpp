#include "graphics/graphics.h"
#include "graphics/vulkan.h"
#include "graphics/objects/gpubuffer.h"
#include "graphics/objects/render/commandlist.h"
#include "graphics/objects/vkgpubuffer.h"
#include "graphics/objects/render/vkcommandlist.h"
using namespace oi::gc;
using namespace oi;

void GPUBuffer::destroy() {

	GraphicsExt &gext = g->getExtension();
	gext.dealloc(*ext, getName());
	
	g->dealloc(ext);

}

GPUBufferExt &GPUBuffer::getExtension() {
	return *ext;
}

bool GPUBufferExt::isVersioned(GPUBufferType type) {
	return type != GPUBufferType::VBO && type != GPUBufferType::IBO;
}

bool GPUBufferExt::isStaged(GPUBufferType type) {
	return type != GPUBufferType::UBO && type != GPUBufferType::CBO && type != GPUBufferType::SSBO;
}

bool GPUBufferExt::isCoherent(GPUBufferType type) {
	return type == GPUBufferType::CBO;
}

void GPUBuffer::flush(Vec2u r) {
	for (Vec2u &c : info.changes)
		c = Vec2u(r.x <= c.x ? r.x : c.x, r.y >= c.y ? r.y : c.y);
}

bool GPUBuffer::shouldStage() {
	return info.changes[g->getExtension().frameId % (u32)info.changes.size()].y != 0 && GPUBufferExt::isVersioned(info.type);
}

bool GPUBuffer::init() {

	ext = g->alloc<GPUBufferExt>();

	GraphicsExt &graphics = g->getExtension();
	info.changes.resize(GPUBufferExt::isVersioned(info.type) ? g->getBuffering() : 1);

	for (Vec2u &change : info.changes)
		change = Vec2u(u32_MAX, 0);

	ext->resource = Array<VkBuffer>(info.changes.size());

	//Create buffer and allocate memory

	VkBufferCreateInfo bufferInfo;
	memset(&bufferInfo, 0, sizeof(bufferInfo));

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = getSize();
	bufferInfo.usage = GPUBufferTypeExt(info.type.getName()).getValue();
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount = 1;
	bufferInfo.pQueueFamilyIndices = &graphics.queueFamilyIndex;

	if (GPUBufferExt::isStaged(info.type))
		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	for (u32 i = 0, j = (u32)ext->resource.size(); i < j; ++i) {
		vkCheck<0x0, GPUBufferExt>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, ext->resource.begin() + i), "Failed to create buffer");
		vkName(graphics, ext->resource[i], VK_OBJECT_TYPE_BUFFER, getName() + " #" + i);
	}

	graphics.alloc(*ext, info.type, getName());

	//Set that it should update

	if(info.hasData)
		set(info.buffer);

	return true;
}

void GPUBuffer::push() {

	u32 frame = g->getExtension().frameId % (u32) ext->resource.size();
	Vec2u &changes = info.changes[frame];

	if (changes.y == 0)
		return;

	u32 off = changes.x;
	u32 len = changes.y - off;

	VkBuffer &resource = ext->resource[frame];
	GraphicsExt &graphics = g->getExtension();

	auto balloc = ext->allocations[frame];

	if (balloc.mappedMemory.size() == 0) {			//Staging

		CommandListExt &cmdList = graphics.stagingCmdList->getExtension();

		//Transition to read write

		VkBufferMemoryBarrier barrier;
		memset(&barrier, 0, sizeof(barrier));

		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.buffer = resource;
		barrier.offset = off;
		barrier.size = len;

		vkCmdPipelineBarrier(cmdList.cmd(graphics), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);

		//Create staging buffer

		GPUBufferExt stagingBuffer;
		stagingBuffer.resource = Array<VkBuffer>(1);

		VkBufferCreateInfo bufferInfo;
		memset(&bufferInfo, 0, sizeof(bufferInfo));

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = len;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 1;
		bufferInfo.pQueueFamilyIndices = &graphics.queueFamilyIndex;

		vkCheck<0x1, GPUBuffer>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, stagingBuffer.resource.begin()), "Failed to create staging buffer");
		vkName(graphics, stagingBuffer.resource[0], VK_OBJECT_TYPE_BUFFER, getName() + " staging buffer");

		graphics.alloc(stagingBuffer, info.type, getName() + " staging buffer", true);

		//Copy data to staging buffer

		balloc = stagingBuffer.allocations[0];
		memcpy(balloc.mappedMemory.addr(), getAddress() + off, len);

		//Copy staging buffer

		VkBufferCopy region = { 0, off, len };
		vkCmdCopyBuffer(cmdList.cmd(graphics), stagingBuffer.resource[0], resource, 1, &region);

		//Transition back to read only

		VkPipelineStageFlagBits stage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		VkAccessFlags access = VK_ACCESS_SHADER_READ_BIT;

		if (info.type == GPUBufferType::IBO)
			access = VK_ACCESS_INDEX_READ_BIT;
		else if (info.type == GPUBufferType::VBO)
			access = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		else
			stage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = access;

		vkCmdPipelineBarrier(cmdList.cmd(graphics), VK_PIPELINE_STAGE_TRANSFER_BIT, stage, 0, 0, nullptr, 1, &barrier, 0, nullptr);

		graphics.stagingBuffers[graphics.frameId][getName() + " staging buffer"] = stagingBuffer;

		changes = Vec2u(u32_MAX, 0);
		return;
	}

	//Copy to memory
	memcpy(balloc.mappedMemory.addr() + off, getAddress() + off, len);

	//Update required if it's non coherent
	if ((balloc.block->memoryBits & (u32)VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {

		u32 boffset = balloc.offset;

		VkDeviceMemory mem = balloc.block->memory;

		u32 offsetAlignment = (u32)graphics.pproperties.properties.limits.nonCoherentAtomSize;
		u32 mapOffset = off / offsetAlignment * offsetAlignment;
		u32 dif = off - mapOffset;

		len += dif;

		VkDeviceSize mapLength = len % ext->alignment == 0 ? len : (len / ext->alignment + 1) * ext->alignment;

		VkMappedMemoryRange range = {
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			nullptr,
			mem,
			mapOffset + boffset,
			mapLength
		};

		vkFlushMappedMemoryRanges(g->getExtension().device, 1, &range);
	}

	changes = Vec2u(u32_MAX, 0);

}