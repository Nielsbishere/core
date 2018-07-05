#pragma once

#include "types/generic.h"
#include "types/buffer.h"

namespace oi {

	struct BlockAllocation {

		u32 start, size;

		u32 end() const { return start + size; }

	};

	//Virtual block allocator; handles object allocation, not memory per se
	class VirtualBlockAllocator {

	public:

		VirtualBlockAllocator(u32 length);

		BlockAllocation alloc(u32 length);
		bool dealloc(u32 pos);

		u32 size();

	protected:

		std::vector<BlockAllocation> blocks, allocations;

		void merge(BlockAllocation allocation);

	private:

		u32 length;

	};

	//A generic block allocator; handles memory
	class BlockAllocator : VirtualBlockAllocator {

	public:

		BlockAllocator(Buffer buffer);
		~BlockAllocator();

		Buffer alloc(u32 size);
		bool dealloc(u8 *ptr);

		template<typename T>
		T *alloc() {
			u8 *addr = alloc(sizeof(T)).addr();
			return ::new(addr) T();
		}

		template<typename T>
		bool dealloc(T *t) {
			t->~T();
			return dealloc((u8*)t);	//TOOD: This ain't work!
		}

		u8 *addr();
		Buffer getBuffer();

	private:

		Buffer buffer;

	};

}