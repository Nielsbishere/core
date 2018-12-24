#ifdef __VULKAN__
#include "graphics/graphics.h"
#include "graphics/objects/gbuffer.h"
#include "graphics/objects/render/commandlist.h"
using namespace oi::gc;
using namespace oi;

GBuffer::~GBuffer() {

	GraphicsExt &gext = g->getExtension();

	info.buffer.deconstruct();

	for(VkBuffer &resource : ext.resource)
		vkDestroyBuffer(gext.device, resource, vkAllocator);

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

void GBuffer::flush(Vec2u r) {
	for (Vec2u &c : info.changes)
		c = Vec2u(r.x <= c.x ? r.x : c.x, r.y >= c.y ? r.y : c.y);
}

bool GBuffer::shouldStage() {
	return info.changes[g->getExtension().current % (u32)info.changes.size()].y != 0 && VkGBuffer::isVersioned(info.type);
}

bool GBuffer::init() {

	GraphicsExt &graphics = g->getExtension();
	info.changes.resize(VkGBuffer::isVersioned(info.type) ? g->getBuffering() : 1);

	for (Vec2u &change : info.changes)
		change = Vec2u(u32_MAX, 0);

	ext.resource.resize(info.changes.size());

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

	for (u32 i = 0, j = (u32)ext.resource.size(); i < j; ++i) {
		vkCheck<0x2, VkGBuffer>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, ext.resource.data() + i), "Failed to create buffer");
		vkName(graphics, ext.resource[i], VK_OBJECT_TYPE_BUFFER, getName() + " #" + i);
	}

	VkMemoryPropertyFlagBits alloc = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;	//CBOs and SSBOs are host visible for quicker access from CPU

	if (VkGBuffer::isStaged(info.type))										//VBOs and IBOs are rarely write, mostly read; so device local
		alloc = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	else if (VkGBuffer::isCoherent(info.type))								//UBOs are frequently unmapped; so should be placed in coherent memory
		alloc = (VkMemoryPropertyFlagBits)(alloc | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	//Allocate memory (TODO: by GraphicsExt)

	VkMemoryAllocateInfo memoryInfo;
	memset(&memoryInfo, 0, sizeof(memoryInfo));

	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(graphics.device, ext.resource[0], &requirements);
	ext.gpuAlignment = u32(std::ceil((f64) requirements.size / requirements.alignment) * requirements.alignment);
	
	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.allocationSize = ext.gpuAlignment * ext.resource.size();
	
	uint32_t memoryIndex = u32_MAX;
	
	for (uint32_t i = 0; i < graphics.pmemory.memoryTypeCount; ++i)
		if ((requirements.memoryTypeBits & (1 << i)) && (graphics.pmemory.memoryTypes[i].propertyFlags & alloc) != 0) {
			memoryIndex = i;
			break;
		}
	
	if (memoryIndex == u32_MAX)
		Log::throwError<VkGBuffer, 0x0>(String("Couldn't find a valid memory type for a VkGBuffer: ") + getName());
	
	memoryInfo.memoryTypeIndex = memoryIndex;
	
	vkCheck<0x3, VkGBuffer>(vkAllocateMemory(graphics.device, &memoryInfo, vkAllocator, &ext.memory), "Couldn't allocate memory");

	for(u32 i = 0, j = (u32) ext.resource.size(); i < j; ++i){
		vkGetBufferMemoryRequirements(graphics.device, ext.resource[i], &requirements);		//Some devices require the requirements to be checked
		vkCheck<0x4, VkGBuffer>(vkBindBufferMemory(graphics.device, ext.resource[i], ext.memory, ext.gpuAlignment * i), String("Couldn't bind memory to buffer ") + getName() + " #" + i);
	}

	//Set that it should update

	if(info.hasData)
		set(info.buffer);

	return true;
}

void GBuffer::push() {

	u32 frame = g->getExtension().current % (u32) ext.resource.size();
	Vec2u &changes = info.changes[frame];

	if (changes.y == 0)
		return;

	u32 off = changes.x;
	u32 len = changes.y - off;

	VkBuffer &resource = ext.resource[frame];
	GraphicsExt &graphics = g->getExtension();

	if (VkGBuffer::isStaged(info.type)) {

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

		VkGBuffer stagingBuffer;
		stagingBuffer.resource.resize(1);

		VkBufferCreateInfo bufferInfo;
		memset(&bufferInfo, 0, sizeof(bufferInfo));

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = len;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 1;
		bufferInfo.pQueueFamilyIndices = &graphics.queueFamilyIndex;

		vkCheck<0x5, GBuffer>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, stagingBuffer.resource.data()), "Failed to create staging buffer");
		vkName(graphics, stagingBuffer.resource[0], VK_OBJECT_TYPE_BUFFER, getName() + " staging buffer");

		//Allocate memory (TODO: by GraphicsExt)

		VkMemoryAllocateInfo memoryInfo;
		memset(&memoryInfo, 0, sizeof(memoryInfo));

		VkMemoryRequirements requirements;
		vkGetBufferMemoryRequirements(graphics.device, stagingBuffer.resource[0], &requirements);

		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.allocationSize = requirements.size;

		uint32_t memoryIndex = u32_MAX;

		for (uint32_t i = 0; i < graphics.pmemory.memoryTypeCount; ++i)
			if ((requirements.memoryTypeBits & (1 << i)) && (graphics.pmemory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
				memoryIndex = i;
				break;
			}

		if (memoryIndex == u32_MAX)
			Log::throwError<VkGBuffer, 0x1>(String("Couldn't find a valid memory type for a staging buffer for VkGBuffer: ") + getName());

		memoryInfo.memoryTypeIndex = memoryIndex;

		vkCheck<0x6, VkGBuffer>(vkAllocateMemory(graphics.device, &memoryInfo, vkAllocator, &stagingBuffer.memory), "Couldn't allocate memory");
		vkCheck<0x7, VkGBuffer>(vkBindBufferMemory(graphics.device, stagingBuffer.resource[0], stagingBuffer.memory, 0), String("Couldn't bind memory to buffer ") + getName());

		//Copy data to staging buffer

		u8 *ptr;
		vkCheck<0x8, VkGBuffer>(vkMapMemory(graphics.device, stagingBuffer.memory, 0, len, 0, (void**)&ptr), "Failed to map staging buffer");

		memcpy(ptr, getAddress() + off, len);

		vkUnmapMemory(graphics.device, stagingBuffer.memory);

		//Copy staging buffer

		VkBufferCopy region = { 0, off, len };
		vkCmdCopyBuffer(cmdList.cmd(graphics), stagingBuffer.resource[0], resource, 1, &region);

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

		graphics.stagingBuffers[graphics.current].push_back(stagingBuffer);

		changes = Vec2u(u32_MAX, 0);
		return;
	}

	u32 boffset = ext.gpuAlignment * graphics.current;

	//Map memory
	u8 *addr;
	vkCheck<0x9, GBuffer>(vkMapMemory(g->getExtension().device, ext.memory, off + boffset, len, 0, (void**)&addr), "Couldn't map memory");

	//Copy buffer
	memcpy(addr, getAddress() + off, len);

	//Flush (if needed)

	if (!VkGBuffer::isCoherent(info.type)) {

		VkMappedMemoryRange range = {
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			nullptr,
			ext.memory,
			off + boffset,
			len
		};

		//TODO: Align this properly
		vkFlushMappedMemoryRanges(g->getExtension().device, 1, &range);
	}

	//Unmap memory
	vkUnmapMemory(g->getExtension().device, ext.memory);

	changes = Vec2u(u32_MAX, 0);

}

#endif