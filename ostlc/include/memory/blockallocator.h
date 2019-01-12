#pragma once

#include "types/generic.h"
#include "types/buffer.h"

namespace oi {

	struct BlockAllocation {

		u32 start, size;

		u32 end() const;
		bool operator==(const BlockAllocation &other) const;

	};

	//Virtual block allocator; handles object allocation, not memory per se
	class VirtualBlockAllocator {

	public:

		VirtualBlockAllocator(u32 length);

		BlockAllocation alloc(u32 length);
		bool dealloc(u32 pos);

		BlockAllocation allocAligned(u32 length, u32 alignment, u32 &alignedStart);

		bool hasSpace(u32 length) const;
		bool hasAlignedSpace(u32 length, u32 alignment) const;

		u32 size() const;
		u32 getAllocations() const;

	protected:

		std::vector<BlockAllocation> blocks, allocations;

		void merge(BlockAllocation allocation);

	private:

		u32 length;

	};

	//A generic block allocator; handles memory
	class BlockAllocator : public VirtualBlockAllocator {

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

		template<typename T, typename ...args>
		T *alloc(args... arg) {
			u8 *addr = alloc(sizeof(T)).addr();
			return ::new(addr) T(arg...);
		}

		template<typename T>
		bool dealloc(T *t) {
			t->~T();
			return dealloc((u8*)t);
		}

		u8 *addr();
		Buffer getBuffer();

	private:

		Buffer buffer;

	};

}