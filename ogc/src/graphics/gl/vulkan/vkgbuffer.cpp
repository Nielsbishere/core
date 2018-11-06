#include "graphics/graphics.h"
#include "graphics/objects/gbuffer.h"
using namespace oi::gc;
using namespace oi;

GBuffer::~GBuffer() {

	GraphicsExt &gext = g->getExtension();

	if(info.ptr != nullptr)
		vkUnmapMemory(gext.device, ext.memory);

	vkDestroyBuffer(gext.device, ext.resource, vkAllocator);
	vkFreeMemory(gext.device, ext.memory, vkAllocator);

}

void GBuffer::open() {
	vkCheck<0x2, GBuffer>(vkMapMemory(g->getExtension().device, ext.memory, 0, info.size, 0, (void**)&info.ptr), "Couldn't map memory");
}

void GBuffer::close() {
	vkUnmapMemory(g->getExtension().device, ext.memory);
	info.ptr = nullptr;
}

bool GBuffer::init() {

	GraphicsExt &graphics = g->getExtension();

	VkBufferCreateInfo bufferInfo;
	memset(&bufferInfo, 0, sizeof(bufferInfo));

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = info.size;
	bufferInfo.usage = GBufferTypeExt(info.type.getName()).getValue();
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount = 1;
	bufferInfo.pQueueFamilyIndices = &graphics.queueFamilyIndex;

	vkCheck<0x0, GBuffer>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, &ext.resource), "Failed to create buffer");

	vkAllocate(Buffer, ext, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	u8 *ptr = info.ptr;

	vkCheck<0x1, GBuffer>(vkMapMemory(graphics.device, ext.memory, 0, info.size, 0, (void**) &info.ptr), "Couldn't map memory");

	if (ptr != nullptr)
		memcpy(info.ptr, ptr, info.size);
	else
		memset(info.ptr, 0, info.size);

	info.ptr = nullptr;
	vkUnmapMemory(graphics.device, ext.memory);

	return true;
}