#include "Types/BufferAllocator.h"
using namespace oi;

bool BufferAllocationObject::operator==(const BufferAllocationObject &other) const {
	return size == other.size && index == other.index;
}

BufferAllocator::BufferAllocator(Buffer _buf, bool _ownBuffer) : buf(_buf), ownBuffer(_ownBuffer) {
	freeSpaces.push_back({ 0, buf.size() });
}

BufferAllocator::BufferAllocator(u32 size): BufferAllocator(Buffer(size)) {}

BufferAllocator::~BufferAllocator() {
	if (ownBuffer)
		buf.deconstruct();
}

Buffer BufferAllocator::getBuffer() { return buf; }

BufferAllocationObject BufferAllocator::allocate(u32 siz) {

	for (u32 i = 0; i < freeSpaces.size(); ++i) {
		auto fs = freeSpaces[i];
		
		if (fs.size > siz) {

			auto fs2 = fs;
			fs2.size = siz;

			fs.index += siz;
			fs.size -= siz;

			freeSpaces.erase(freeSpaces.begin() + i);
			freeSpaces.push_back(fs);
			allocs.push_back(fs2);
			return fs;

		} else if (fs.size == siz) {
			freeSpaces.erase(freeSpaces.begin() + i);
			allocs.push_back(fs);
			return fs;
		}
	}

	return { 0, 0 };
}

bool BufferAllocator::deallocate(BufferAllocationObject bao) {

	for (u32 i = 0; i < allocs.size(); ++i) {

		if (bao == allocs[i]) {

			allocs.erase(allocs.begin() + i);
			freeSpaces.push_back(bao);

			return false;
		}
	}
	
	return true;
}

u32 BufferAllocator::size() { return allocs.size(); }
u32 BufferAllocator::getBufferSize() { return buf.size(); }