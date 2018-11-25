#ifdef __VULKAN__
#include "graphics/graphics.h"
#include "graphics/objects/gbuffer.h"
#include "graphics/objects/render/commandlist.h"
using namespace oi::gc;
using namespace oi;

GBuffer::~GBuffer() {

	GraphicsExt &gext = g->getExtension();

	info.buffer.deconstruct();
	vkDestroyBuffer(gext.device, ext.resource, vkAllocator);
	vkFreeMemory(gext.device, ext.memory, vkAllocator);

}

bool VkGBuffer::isVersioned(GBufferType type) {
	return type != GBufferType::VBO && type != GBufferType::IBO;
}

bool VkGBuffer::isStaged(GBufferType type) {
	return type != GBufferType::UBO && type != GBufferType::CBO && type != GBufferType::SSBO;
}

bool VkGBuffer::isCoherent(GBufferType type) {
	return type == GBufferType::CBO;
}

void GBuffer::flush() {

	if (VkGBuffer::isStaged(info.type)) {

		GraphicsExt &graphics = g->getExtension();
		CommandListExt &cmdList = graphics.stagingCmdList->getExtension();

		//Transition to read write

		VkBufferMemoryBarrier barrier;
		memset(&barrier, 0, sizeof(barrier));

		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.buffer = ext.resource;
		barrier.size = getSize();

		vkCmdPipelineBarrier(cmdList.cmd(graphics), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);

		//Create staging buffer

		VkGBuffer stagingBuffer;

		VkBufferCreateInfo bufferInfo;
		memset(&bufferInfo, 0, sizeof(bufferInfo));

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = getSize();
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 1;
		bufferInfo.pQueueFamilyIndices = &graphics.queueFamilyIndex;

		vkCheck<0x0, GBuffer>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, &stagingBuffer.resource), "Failed to create staging buffer");
		vkName(graphics, stagingBuffer.resource, VK_OBJECT_TYPE_BUFFER, getName() + " staging buffer");

		vkAllocate(Buffer, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		//Copy data to staging buffer

		u8 *ptr;
		vkCheck<0x2, GBuffer>(vkMapMemory(graphics.device, stagingBuffer.memory, 0, getSize(), 0, (void**)&ptr), "Failed to map staging buffer");

		memcpy(ptr, getAddress(), getSize());

		vkUnmapMemory(graphics.device, stagingBuffer.memory);

		//Copy staging buffer

		VkBufferCopy region = { 0, 0, getSize() };
		vkCmdCopyBuffer(cmdList.cmd(graphics), stagingBuffer.resource, ext.resource, 1, &region);

		//Transition back to read only

		VkPipelineStageFlagBits stage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		VkAccessFlags access = VK_ACCESS_SHADER_READ_BIT;

		if (info.type == GBufferType::IBO)
			access = VK_ACCESS_INDEX_READ_BIT;
		else if (info.type == GBufferType::VBO)
			access = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		else
			stage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = access;

		vkCmdPipelineBarrier(cmdList.cmd(graphics), VK_PIPELINE_STAGE_TRANSFER_BIT, stage, 0, 0, nullptr, 1, &barrier, 0, nullptr);

		graphics.stagingBuffers.push_back(stagingBuffer);

		return;
	}

	//Map memory
	u8 *addr;
	vkCheck<0x2, GBuffer>(vkMapMemory(g->getExtension().device, ext.memory, 0, getSize(), 0, (void**)&addr), "Couldn't map memory");

	//Copy buffer
	memcpy(addr, getAddress(), getSize());

	//Flush (if needed)

	if (!VkGBuffer::isCoherent(info.type)) {

		VkMappedMemoryRange range = {
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			nullptr,
			ext.memory,
			0,
			VK_WHOLE_SIZE
		};

		vkFlushMappedMemoryRanges(g->getExtension().device, 1, &range);
	}

	//Unmap memory
	vkUnmapMemory(g->getExtension().device, ext.memory);

}

bool GBuffer::init() {

	GraphicsExt &graphics = g->getExtension();

	//Create buffer and allocate memory

	VkBufferCreateInfo bufferInfo;
	memset(&bufferInfo, 0, sizeof(bufferInfo));

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = getSize();
	bufferInfo.usage = GBufferTypeExt(info.type.getName()).getValue();
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount = 1;
	bufferInfo.pQueueFamilyIndices = &graphics.queueFamilyIndex;

	if (VkGBuffer::isStaged(info.type))
		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	vkCheck<0x1, GBuffer>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, &ext.resource), "Failed to create buffer");
	vkName(graphics, ext.resource, VK_OBJECT_TYPE_BUFFER, getName());

	VkMemoryPropertyFlagBits alloc = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;	//CBOs and SSBOs are host visible for quicker access from CPU

	if (VkGBuffer::isStaged(info.type))									//VBOs and IBOs are rarely write, mostly read; so device local
		alloc = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	else if (VkGBuffer::isCoherent(info.type))								//UBOs are frequently unmapped; so should be placed in coherent memory
		alloc = (VkMemoryPropertyFlagBits)(alloc | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vkAllocate(Buffer, ext, alloc);

	if(info.hasData)
		set(info.buffer);

	return true;
}

#endif