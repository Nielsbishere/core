#include "memory/blockallocator.h"
using namespace oi;

u32 BlockAllocation::end() const { return start + size; }
bool BlockAllocation::operator==(const BlockAllocation &other) const { return start == other.start && size == other.size; }

VirtualBlockAllocator::VirtualBlockAllocator(u32 length) : length(length) { blocks.push_back({ 0, length }); }

BlockAllocation VirtualBlockAllocator::alloc(u32 size) {

	BlockAllocation result = { 0, 0 };
	u32 i = 0;

	for (BlockAllocation block : blocks)
		if (block.size == size) {
			blocks.erase(blocks.begin() + i);
			allocations.push_back(result = { block.start, size });
			break;
		}
		else if (block.size > size) {
			blocks[i] = { block.start + size, block.size - size };
			allocations.push_back(result = { block.start, size });
			break;
		}
		else
			++i;

	return result;
}

BlockAllocation VirtualBlockAllocator::allocAligned(u32 size, u32 alignment, u32 &alignedStart) {

	BlockAllocation result = { 0, 0 };
	u32 i = 0;

	for (BlockAllocation block : blocks) {

		u32 aligned = u32(std::ceil((f64)block.start / alignment) * alignment);
		u32 alignedEnd = aligned + size;
		u32 alignedSize = alignedEnd - block.start;

		if (block.size == alignedSize) {
			blocks.erase(blocks.begin() + i);
			allocations.push_back(result = { block.start, alignedSize });
			alignedStart = aligned;
			break;
		}
		else if (block.size > alignedSize) {
			blocks[i] = { alignedEnd, block.size - alignedSize };
			allocations.push_back(result = { block.start, alignedSize });
			alignedStart = aligned;
			break;
		}
		else
			++i;
	}

	return result;
}

bool VirtualBlockAllocator::hasSpace(u32 size) const {

	for (BlockAllocation block : blocks)
		if (block.size >= size)
			return true;

	return false;
}

bool VirtualBlockAllocator::hasAlignedSpace(u32 size, u32 alignment) const {

	for (BlockAllocation block : blocks) {
		u32 alignedStart = u32(std::ceil((f64)block.start / alignment) * alignment);
		if (alignedStart + size < block.end())
			return true;
	}

	return false;
}

bool VirtualBlockAllocator::dealloc(u32 pos) {

	u32 i = 0;

	for (BlockAllocation block : allocations)
		if (block.start == pos) {
			allocations.erase(allocations.begin() + i);
			merge(block);
			return true;
		}
		else ++i;

	return false;
}

void VirtualBlockAllocator::merge(BlockAllocation allocation) {

	BlockAllocation *left = nullptr, *right = nullptr;

	for (BlockAllocation &block : blocks)
		if (allocation.end() == block.start)
			right = &block;
		else if (allocation.start == block.end())
			left = &block;
		else if (left != nullptr && right != nullptr)
			break;

	if (left)
		left->size += allocation.size;

	if (left && right) {
		left->size += right->size;
		blocks.erase(blocks.begin() + (right - blocks.data()));
	}
	else if (right) {
		right->size += allocation.size;
		right->start = allocation.start;
	}
	else if (!left)
		blocks.push_back(allocation);

}

u32 VirtualBlockAllocator::size() const { return length; }
u32 VirtualBlockAllocator::getAllocations() const { return (u32) allocations.size(); }

BlockAllocator::BlockAllocator(Buffer buffer) : VirtualBlockAllocator(buffer.size()), buffer(buffer) { }
BlockAllocator::~BlockAllocator() { 
	buffer.deconstruct(); 
}

Buffer BlockAllocator::alloc(u32 size) {

	BlockAllocation ba = VirtualBlockAllocator::alloc(size);

	if (ba.size == 0)
		return {};

	return Buffer::construct(addr() + ba.start, ba.size);
}

bool BlockAllocator::dealloc(u8 *ptr) {

	if (ptr < buffer.addr() && ptr >= buffer.addr() + buffer.size())
		return false;

	u32 offset = u32(ptr - buffer.addr());
	return VirtualBlockAllocator::dealloc(offset);
}

u8 *BlockAllocator::addr() { return buffer.addr(); }
Buffer BlockAllocator::getBuffer() { return buffer; }