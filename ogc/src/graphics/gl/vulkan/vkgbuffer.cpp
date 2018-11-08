#ifdef __VULKAN__
#include "graphics/graphics.h"
#include "graphics/objects/gbuffer.h"
using namespace oi::gc;
using namespace oi;

GBuffer::~GBuffer() {

	GraphicsExt &gext = g->getExtension();

	info.buffer.deconstruct();
	vkDestroyBuffer(gext.device, ext.resource, vkAllocator);
	vkFreeMemory(gext.device, ext.memory, vkAllocator);

}

void GBuffer::flush() {

	//Map memory
	u8 *addr;
	vkCheck<0x2, GBuffer>(vkMapMemory(g->getExtension().device, ext.memory, 0, getSize(), 0, (void**)&addr), "Couldn't map memory");

	//Copy buffer
	memcpy(addr, getAddress(), getSize());

	//Flush (if needed)

	if (info.type == GBufferType::UBO) {

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

	vkCheck<0x0, GBuffer>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, &ext.resource), "Failed to create buffer");
	vkName(graphics, ext.resource, VK_OBJECT_TYPE_BUFFER, getName());

	VkMemoryPropertyFlagBits alloc = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	if (info.type == GBufferType::UBO)	//UBO's are frequently unmapped; so should be placed in coherent memory
		alloc = (VkMemoryPropertyFlagBits)(alloc | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vkAllocate(Buffer, ext, alloc);

	//Update data

	u8 *addr;
	vkCheck<0x1, GBuffer>(vkMapMemory(graphics.device, ext.memory, 0, getSize(), 0, (void**) &addr), "Couldn't map memory");

	memcpy(addr, getAddress(), getSize());

	if (info.type == GBufferType::UBO) {

		VkMappedMemoryRange range = {
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			nullptr,
			ext.memory,
			0,
			VK_WHOLE_SIZE
		};

		vkFlushMappedMemoryRanges(graphics.device, 1, &range);
	}

	vkUnmapMemory(graphics.device, ext.memory);

	return true;
}

#endif