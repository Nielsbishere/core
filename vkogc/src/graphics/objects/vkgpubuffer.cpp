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

	for(VkBuffer &resource : ext->resource)
		vkDestroyBuffer(gext.device, resource, vkAllocator);

	vkFreeMemory(gext.device, ext->memory, vkAllocator);
	
	g->dealloc<GPUBuffer>(ext);

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
	return info.changes[g->getExtension().current % (u32)info.changes.size()].y != 0 && GPUBufferExt::isVersioned(info.type);
}

bool GPUBuffer::init() {

	g->alloc<GPUBuffer>(ext);

	GraphicsExt &graphics = g->getExtension();
	info.changes.resize(GPUBufferExt::isVersioned(info.type) ? g->getBuffering() : 1);

	for (Vec2u &change : info.changes)
		change = Vec2u(u32_MAX, 0);

	ext->resource.resize(info.changes.size());

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
		vkCheck<0x2, GPUBufferExt>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, ext->resource.data() + i), "Failed to create buffer");
		vkName(graphics, ext->resource[i], VK_OBJECT_TYPE_BUFFER, getName() + " #" + i);
	}

	VkMemoryPropertyFlagBits alloc = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;	//CBOs and SSBOs are host visible for quicker access from CPU

	if (GPUBufferExt::isStaged(info.type))										//VBOs and IBOs are rarely write, mostly read; so device local
		alloc = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	else if (GPUBufferExt::isCoherent(info.type))								//UBOs are frequently unmapped; so should be placed in coherent memory
		alloc = (VkMemoryPropertyFlagBits)(alloc | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	//Allocate memory (TODO: by GraphicsExt)

	VkMemoryAllocateInfo memoryInfo;
	memset(&memoryInfo, 0, sizeof(memoryInfo));

	VkMemoryRequirements2 requirements2;
	memset(&requirements2, 0, sizeof(requirements2));

	requirements2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR;

	VkBufferMemoryRequirementsInfo2 bufferMemoryRequirements2;
	memset(&bufferMemoryRequirements2, 0, sizeof(bufferMemoryRequirements2));

	bufferMemoryRequirements2.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR;
	bufferMemoryRequirements2.buffer = ext->resource[0];

	graphics.vkGetBufferMemoryRequirements2(graphics.device, &bufferMemoryRequirements2, &requirements2);
	VkMemoryRequirements &requirements = requirements2.memoryRequirements;

	ext->size = (u32) requirements.size;
	ext->alignment = (u32) requirements.alignment;
	ext->alignedSize = u32(std::ceil((f64) ext->size / ext->alignment) * ext->alignment);
	
	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.allocationSize = ext->alignedSize * ext->resource.size();
	
	uint32_t memoryIndex = u32_MAX;
	
	for (uint32_t i = 0; i < graphics.pmemory.memoryTypeCount; ++i)
		if ((requirements.memoryTypeBits & (1 << i)) && (graphics.pmemory.memoryTypes[i].propertyFlags & alloc) != 0) {
			memoryIndex = i;
			break;
		}
	
	if (memoryIndex == u32_MAX)
		Log::throwError<GPUBufferExt, 0x0>(String("Couldn't find a valid memory type for a VkGPUBuffer: ") + getName());
	
	memoryInfo.memoryTypeIndex = memoryIndex;
	
	vkCheck<0x3, GPUBufferExt>(vkAllocateMemory(graphics.device, &memoryInfo, vkAllocator, &ext->memory), "Couldn't allocate memory");

	for(u32 i = 0, j = (u32) ext->resource.size(); i < j; ++i){

		if (i != 0) {			//Always get memory requirements, even though we already know them (as they are identical to the first)
			bufferMemoryRequirements2.buffer = ext->resource[i];
			graphics.vkGetBufferMemoryRequirements2(graphics.device, &bufferMemoryRequirements2, &requirements2);
		}

		vkCheck<0x4, GPUBufferExt>(vkBindBufferMemory(graphics.device, ext->resource[i], ext->memory, ext->alignedSize * i), String("Couldn't bind memory to buffer ") + getName() + " #" + i);
	}

	//Set that it should update

	if(info.hasData)
		set(info.buffer);

	return true;
}

void GPUBuffer::push() {

	u32 frame = g->getExtension().current % (u32) ext->resource.size();
	Vec2u &changes = info.changes[frame];

	if (changes.y == 0)
		return;

	u32 off = changes.x;
	u32 len = changes.y - off;

	VkBuffer &resource = ext->resource[frame];
	GraphicsExt &graphics = g->getExtension();

	if (GPUBufferExt::isStaged(info.type)) {

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
		stagingBuffer.resource.resize(1);

		VkBufferCreateInfo bufferInfo;
		memset(&bufferInfo, 0, sizeof(bufferInfo));

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = len;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 1;
		bufferInfo.pQueueFamilyIndices = &graphics.queueFamilyIndex;

		vkCheck<0x5, GPUBuffer>(vkCreateBuffer(graphics.device, &bufferInfo, vkAllocator, stagingBuffer.resource.data()), "Failed to create staging buffer");
		vkName(graphics, stagingBuffer.resource[0], VK_OBJECT_TYPE_BUFFER, getName() + " staging buffer");

		//Allocate memory (TODO: by GraphicsExt)

		VkMemoryAllocateInfo memoryInfo;
		memset(&memoryInfo, 0, sizeof(memoryInfo));


		VkMemoryRequirements2 requirements2;
		memset(&requirements2, 0, sizeof(requirements2));

		requirements2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR;

		VkBufferMemoryRequirementsInfo2 bufferMemoryRequirements2;
		memset(&bufferMemoryRequirements2, 0, sizeof(bufferMemoryRequirements2));

		bufferMemoryRequirements2.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR;
		bufferMemoryRequirements2.buffer = stagingBuffer.resource[0];

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
			Log::throwError<GPUBufferExt, 0x1>(String("Couldn't find a valid memory type for a staging buffer for VkGPUBuffer: ") + getName());

		memoryInfo.memoryTypeIndex = memoryIndex;

		vkCheck<0x6, GPUBufferExt>(vkAllocateMemory(graphics.device, &memoryInfo, vkAllocator, &stagingBuffer.memory), "Couldn't allocate memory");
		vkCheck<0x7, GPUBufferExt>(vkBindBufferMemory(graphics.device, stagingBuffer.resource[0], stagingBuffer.memory, 0), String("Couldn't bind memory to buffer ") + getName());

		//Copy data to staging buffer

		u8 *ptr;
		vkCheck<0x8, GPUBufferExt>(vkMapMemory(graphics.device, stagingBuffer.memory, 0, len, 0, (void**)&ptr), "Failed to map staging buffer");

		memcpy(ptr, getAddress() + off, len);

		vkUnmapMemory(graphics.device, stagingBuffer.memory);

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

		graphics.stagingBuffers[graphics.current].push_back(stagingBuffer);

		changes = Vec2u(u32_MAX, 0);
		return;
	}

	u32 boffset = ext->alignedSize * graphics.current;

	u32 offsetAlignment = (u32) graphics.pproperties.properties.limits.nonCoherentAtomSize;
	u32 mapOffset = off / offsetAlignment * offsetAlignment;
	u32 dif = off - mapOffset;

	len += dif;

	VkDeviceSize mapLength = len % ext->alignment == 0 ? len : (len / ext->alignment + 1) * ext->alignment;

	//Map memory
	u8 *addr;
	vkCheck<0x9, GPUBuffer>(vkMapMemory(g->getExtension().device, ext->memory, mapOffset + boffset, mapLength, 0, (void**)&addr), "Couldn't map memory");

	//Copy buffer
	memcpy(addr + dif, getAddress() + off, len);

	//Flush (if needed)

	if (!GPUBufferExt::isCoherent(info.type)) {

		VkMappedMemoryRange range = {
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			nullptr,
			ext->memory,
			mapOffset + boffset,
			mapLength
		};

		vkFlushMappedMemoryRanges(g->getExtension().device, 1, &range);
	}

	//Unmap memory
	vkUnmapMemory(g->getExtension().device, ext->memory);

	changes = Vec2u(u32_MAX, 0);

}