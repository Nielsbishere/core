#include "graphics/objects/gpubuffer.h"
#include "graphics/objects/texture/texture.h"
#include "graphics/vulkan.h"
#include "graphics/vkgraphics.h"
#include "graphics/vkgpuallocator.h"
#include "graphics/objects/vkgpubuffer.h"
#include "graphics/objects/texture/vktexture.h"
using namespace oi::gc;
using namespace oi;

bool GPUMemoryBlockExt::compatible(const std::tuple<VkMemoryPropertyFlagBits, VkMemoryRequirements, VkMemoryDedicatedRequirementsKHR> &requirements) const {
	return !isDedicated && (memoryBits & std::get<0>(requirements)) == std::get<0>(requirements) &&
		(std::get<1>(requirements).memoryTypeBits & (1 << memoryId)) != 0 &&
		allocator.hasAlignedSpace((u32)std::get<1>(requirements).size, (u32)std::get<1>(requirements).alignment);
}

void GPUMemoryBlockExt::free() {

	if (mappedMemory.size() != 0)
		vkUnmapMemory(g->device, memory);

	vkFreeMemory(g->device, memory, vkAllocator);
}

bool GPUMemoryBlockExt::free(BlockAllocation range) {

	allocator.dealloc(range.start);

	if (allocator.getAllocations() == 0)
		free();

	return allocator.getAllocations() == 0;
}

bool GPUAllocationExt::operator==(const GPUAllocationExt &other) const {
	return block == other.block && offset == other.offset && allocation == other.allocation;
}


GPUMemoryBlockExt *GraphicsExt::alloc(const std::tuple<VkMemoryPropertyFlagBits, VkMemoryRequirements, VkMemoryDedicatedRequirementsKHR> &requirements, String &resourceName, u32 &offset, BlockAllocation &allocation, Buffer &mapped, std::pair<VkImage, VkBuffer> res) {

	VkMemoryPropertyFlagBits allocFlags = std::get<0>(requirements);
	const VkMemoryRequirements &requirements1 = std::get<1>(requirements);
	const VkMemoryDedicatedRequirementsKHR &dedicatedInfo = std::get<2>(requirements);

	bool dedicated = dedicatedInfo.prefersDedicatedAllocation || dedicatedInfo.requiresDedicatedAllocation;

	if (dedicated) {

		VkMemoryAllocateInfo memoryInfo;
		memset(&memoryInfo, 0, sizeof(memoryInfo));

		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.allocationSize = requirements1.size;

		VkMemoryDedicatedAllocateInfoKHR dedicatedAlloc = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR, nullptr, res.first, res.second };
		memoryInfo.pNext = &dedicatedAlloc;

		uint32_t memoryIndex = u32_MAX;

		for (uint32_t i = 0; i < pmemory.memoryTypeCount; ++i)
			if ((requirements1.memoryTypeBits & (1 << i)) && (pmemory.memoryTypes[i].propertyFlags & allocFlags) == (u32)allocFlags) {
				memoryIndex = i;
				break;
			}

		if (memoryIndex == u32_MAX)
			Log::throwError<GraphicsExt, 0x26>("Couldn't find a valid memory type that fits the resource");

		memoryInfo.memoryTypeIndex = memoryIndex;

		VkDeviceMemory memory;
		vkCheck<0x27>(vkAllocateMemory(device, &memoryInfo, vkAllocator, &memory), "Couldn't allocate memory for resource");

		vkName(*this, memory, VK_OBJECT_TYPE_DEVICE_MEMORY, resourceName + " memory");

		allocFlags = (VkMemoryPropertyFlagBits) pmemory.memoryTypes[memoryIndex].propertyFlags;

		if ((allocFlags & (u32)VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {

			u8 *addr = nullptr;
			vkCheck<0x2D>(vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, (void**) &addr), "Couldn't map dedicated memory");
			
			mapped = Buffer::construct(addr, (u32) requirements1.size);

		}

		GPUMemoryBlockExt *block = new GPUMemoryBlockExt{
			this,
			memoryIndex,
			(VkMemoryAllocateFlagBits) pmemory.memoryTypes[memoryIndex].propertyFlags,
			VirtualBlockAllocator((u32)requirements1.size),
			memory,
			mapped,
			true
		};

		memoryBlocks.push_back(block);
		allocation = block->allocator.alloc((u32)requirements1.size);
		offset = 0;
		Log::println(String("Allocated dedicated memory for resource: ") + resourceName + " (" + allocation.size + " bytes mapped at " + String((void*)mapped.addr()) + ")");
		return block;
	}

	std::vector<GPUMemoryBlockExt*>::iterator it;

	for (it = memoryBlocks.begin(); it != memoryBlocks.end(); ++it)
		if ((*it)->compatible(requirements))
			break;

	if (it == memoryBlocks.end()) {

		VkMemoryAllocateInfo memoryInfo;
		memset(&memoryInfo, 0, sizeof(memoryInfo));

		u32 size = requirements1.size > memoryBlockSize ? (u32)requirements1.size : (u32)memoryBlockSize;

		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.allocationSize = size;

		uint32_t memoryIndex = u32_MAX;

		for (uint32_t i = 0; i < pmemory.memoryTypeCount; ++i)
			if ((requirements1.memoryTypeBits & (1 << i)) && (pmemory.memoryTypes[i].propertyFlags & allocFlags) == (u32)allocFlags) {
				memoryIndex = i;
				break;
			}

		if (memoryIndex == u32_MAX)
			Log::throwError<GraphicsExt, 0x28>("Couldn't find a valid memory type");

		memoryInfo.memoryTypeIndex = memoryIndex;

		VkDeviceMemory memory;
		vkCheck<0x29, GraphicsExt>(vkAllocateMemory(device, &memoryInfo, vkAllocator, &memory), "Couldn't allocate memory");

		Buffer mappedMemory;
		allocFlags = (VkMemoryPropertyFlagBits)pmemory.memoryTypes[memoryIndex].propertyFlags;

		if ((allocFlags & (u32)VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {

			u8 *addr = nullptr;
			vkCheck<0x2E>(vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, (void**)&addr), "Couldn't map memory");

			mappedMemory = Buffer::construct(addr, size);
			mapped = mappedMemory.subbuffer(0, (u32)requirements1.size);
		}

		GPUMemoryBlockExt *block = new GPUMemoryBlockExt{
			this,
			memoryIndex,
			(VkMemoryAllocateFlagBits) pmemory.memoryTypes[memoryIndex].propertyFlags,
			VirtualBlockAllocator(size),
			memory,
			mappedMemory
		};

		memoryBlocks.push_back(block);
		allocation = block->allocator.alloc((u32)requirements1.size);
		offset = 0;
		Log::println(String("Allocated memory for resource: ") + resourceName + " (" + allocation.size + " bytes / " + size + " chunk size mapped at " + String((void*)mapped.addr()) + ")");
		return block;
	}

	u32 aliasing = (u32) pproperties.properties.limits.bufferImageGranularity;
	u32 alignment = (u32) requirements1.alignment;

	allocation = (*it)->allocator.allocAligned((u32)requirements1.size, alignment < aliasing ? aliasing : alignment, offset);
	
	Buffer mappedMemory = (*it)->mappedMemory;

	if (mappedMemory.size() != 0)
		mapped = mappedMemory.subbuffer(offset, (u32)requirements1.size);

	Log::println(String("Allocated memory for resource: ") + resourceName + " (" + allocation.size + " bytes at " + offset + " mapped at " + String((void*)mapped.addr()) + ")");
	return memoryBlocks[it - memoryBlocks.begin()];
}

void GraphicsExt::dealloc(GPUMemoryBlockExt *block, BlockAllocation allocation) {

	Log::println(String("Freeing object at offset ") + allocation.start + " with " + allocation.size + " bytes");

	if (block->free(allocation)) {

		auto it = std::find(memoryBlocks.begin(), memoryBlocks.end(), block);

		if (it != memoryBlocks.end()) {
			memoryBlocks.erase(it);
			Log::println(String("Freeing memory block; ") + block->allocator.size() + " bytes");
			delete block;
		}

	}

}

void GraphicsExt::alloc(GPUBufferExt &ext, GPUBufferType type, String name, bool isStaging) {

	VkMemoryPropertyFlagBits allocFlags =
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;	//CBOs and UBOs are host visible for quicker access from CPU

	if (GPUBufferExt::isStaged(type))			//SSBOs, VBOs and IBOs are rarely write, mostly read; so device local
		allocFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	if (GPUBufferExt::isCoherent(type))			//CBOs are frequently unmapped; so should be placed in coherent memory
		allocFlags = (VkMemoryPropertyFlagBits)(allocFlags | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (isStaging)								//Staging buffers should be host visible & coherent
		allocFlags = (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	for (u32 i = 0, j = (u32)ext.resource.size(); i < j; ++i) {

		VkBuffer &res = ext.resource[i];
		String rname = name + " #" + i;

		VkMemoryDedicatedRequirementsKHR dedicatedReq = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR };
		VkBufferMemoryRequirementsInfo2 bufferReq = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR, nullptr, res };

		VkMemoryRequirements2 memReq = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR, &dedicatedReq };
		vkGetBufferMemoryRequirements2(device, &bufferReq, &memReq);

		VkMemoryRequirements requirements = memReq.memoryRequirements;

		auto allocInfo = std::tuple<VkMemoryPropertyFlagBits, VkMemoryRequirements, VkMemoryDedicatedRequirementsKHR>(allocFlags, requirements, dedicatedReq);

		u32 offset = 0;

		BlockAllocation allocation;
		Buffer mapped;
		GPUMemoryBlockExt *gallocation = alloc(allocInfo, rname, offset, allocation, mapped, { VK_NULL_HANDLE, res });

		if (i == 0)
			ext.alignment = (u32)requirements.alignment;

		ext.allocations.push_back({ gallocation, offset, allocation, mapped });

		vkCheck<0x2A>(vkBindBufferMemory(device, res, gallocation->memory, offset), "Couldn't bind buffer memory");

	}

}

void GraphicsExt::dealloc(GPUBufferExt &ext, String name) {

	for (u32 i = 0, j = (u32)ext.resource.size(); i < j; ++i) {

		VkBuffer buffer = ext.resource[i];
		vkDestroyBuffer(device, buffer, vkAllocator);

		GPUAllocationExt &balloc = ext.allocations[i];
		GPUMemoryBlockExt *memoryBlock = balloc.block;

		Log::println(String("Deallocated ") + name + " #" + i + " at " + balloc.allocation.start + " with size " + balloc.allocation.size);

		if (memoryBlock->free(balloc.allocation)) {
			delete memoryBlock;
			memoryBlocks.erase(std::find(memoryBlocks.begin(), memoryBlocks.end(), memoryBlock));
		}

	}

}

void GraphicsExt::alloc(TextureExt &ext, String name) {

	VkMemoryPropertyFlagBits allocFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	VkImage &res = ext.resource;

	VkMemoryDedicatedRequirementsKHR dedicatedReq = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR };
	VkImageMemoryRequirementsInfo2 imageReq = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2_KHR, nullptr, res };

	VkMemoryRequirements2 memReq = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR, &dedicatedReq };
	vkGetImageMemoryRequirements2(device, &imageReq, &memReq);

	VkMemoryRequirements requirements = memReq.memoryRequirements;

	auto allocInfo = std::tuple<VkMemoryPropertyFlagBits, VkMemoryRequirements, VkMemoryDedicatedRequirementsKHR>(allocFlags, requirements, dedicatedReq);

	u32 offset = 0;

	BlockAllocation allocation;
	Buffer mapped;
	GPUMemoryBlockExt *gallocation = alloc(allocInfo, name, offset, allocation, mapped, { res, VK_NULL_HANDLE });

	ext.allocation = { gallocation, offset, allocation, mapped };

	vkCheck<0x2B>(vkBindImageMemory(device, res, gallocation->memory, offset), "Couldn't bind image memory");

}

void GraphicsExt::dealloc(TextureExt &ext, String name) {

	GPUAllocationExt &balloc = ext.allocation;
	GPUMemoryBlockExt *memoryBlock = balloc.block;

	if (memoryBlock == nullptr)
		return;

	VkImage image = ext.resource;
	vkDestroyImage(device, image, vkAllocator);

	Log::println(String("Deallocated ") + name + " at " + balloc.allocation.start + " with size " + balloc.allocation.size);

	if (memoryBlock->free(balloc.allocation)) {
		delete memoryBlock;
		memoryBlocks.erase(std::find(memoryBlocks.begin(), memoryBlocks.end(), memoryBlock));
	}

}