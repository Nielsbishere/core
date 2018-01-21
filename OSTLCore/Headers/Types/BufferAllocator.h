#pragma once

#include "Buffer.h"

namespace oi {

	struct BufferAllocationObject {
		u32 index, size;

		bool operator==(const BufferAllocationObject &other) const;
	};

	class BufferAllocator {

	public:

		BufferAllocator(Buffer buf, bool ownBuffer = true);
		BufferAllocator(u32 size);
		~BufferAllocator();

		//Returns BAO; 0, 0 if invalid
		BufferAllocationObject allocate(u32 size);

		//Deallocate a BufferAllocationObject (BAO)
		bool deallocate(BufferAllocationObject bao);

		u32 size();
		u32 getBufferSize();

		Buffer getBuffer();

	private:

		Buffer buf;
		bool ownBuffer;

		std::vector<BufferAllocationObject> allocs, freeSpaces;

	};

}