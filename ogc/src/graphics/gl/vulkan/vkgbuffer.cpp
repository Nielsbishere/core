#include "graphics/gbuffer.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

GBuffer::~GBuffer() {

	GraphicsExt &gext = g->getExtension();

	if(info.ptr != nullptr)
		vkUnmapMemory(gext.device, ext.memory);

	vkDestroyBuffer(gext.device, ext.resource, allocator);
	vkFreeMemory(gext.device, ext.memory, allocator);

}

bool GBuffer::set(Buffer buf) {

	if (buf.size() != info.size)
		return Log::error("GBuffer::set please use a buffer that matches the gbuffer's size");

	GraphicsExt &gext = g->getExtension();

	if (info.persistent)
		memcpy(info.ptr, buf.addr(), info.size);
	else {
		vkCheck<0x2, GBuffer>(vkMapMemory(gext.device, ext.memory, 0, info.size, 0, (void**)&info.ptr), "Couldn't map memory");
		memcpy(info.ptr, buf.addr(), info.size);
		vkUnmapMemory(gext.device, ext.memory);

		info.ptr = nullptr;
	}

	return true;
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

	vkCheck<0x0, GBuffer>(vkCreateBuffer(graphics.device, &bufferInfo, allocator, &ext.resource), "Failed to create buffer");

	vkAllocate(Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	u8 *ptr = info.ptr;

	if (info.persistent || info.ptr != nullptr)
		vkCheck<0x1, GBuffer>(vkMapMemory(graphics.device, ext.memory, 0, info.size, 0, (void**) &info.ptr), "Couldn't map memory");

	if(ptr != nullptr)
		memcpy(info.ptr, ptr, info.size);

	if (ptr != nullptr && !info.persistent) {
		info.ptr = nullptr;
		vkUnmapMemory(graphics.device, ext.memory);
	}

	return true;
}