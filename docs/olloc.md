# olloc (Oxsomi malloc implementation)

With oxsomi allocation, every allocation is stored onto a MemoryPage; which has the default size of 128 MiB. When it runs out of memory, it allocates a new MemoryPage. When it tries to allocate something that's bigger than the page size, it creates a custom sized MemoryPage that can fit it. Whenever the entire page is empty, it will deallocate it (unless it is specified to remain allocated). This means that whenever you exceed (or it just doesn't fit in-between memory) the 128 MiB page allocated by default, it will allocate 128 MiB more. There is a limit of 8192 pages (>1 TiB). The allocator can be implemented per platform, making it compatible for console/mobile/desktop development.

## Code structure

```cpp
struct MemoryPage {
  
	static constexpr size_t maxPages = 8192, defaultPageSize = 128_MiB;
    
	u8 *data = nullptr;					//MemoryBlock[]
	size_t size = 0;
	bool stayAllocated = false;
    
    MemoryPage();
	MemoryPage(size_t size, bool stayAllocated = false);
	~MemoryPage();
    
};

//First bit of prev is used to determine if it's occupied
struct MemoryBlock {
    size_t size, prev;
    //u8 data[];
};

//Example:
MemoryPage allocation = {
    
    //ABCD\0 + 16 bytes for alloc (aka, don't put ABCD on heap)
    { 5 + sizeof(MemoryBlock), -1, "ABCD" },
    
    //All other bytes are free
    { 128_MiB - 5 - sizeof(MemoryBlock), 5, ... }
};
```

The field "allocation" has a String called "ABCD\0" allocated into the first memory block, while the second block is the 'free' block, the block has prev set to -1; meaning it is occupied, and since `~-1` = `0`, it doesn't have any previous block. The block after is the rest of the allocation and isn't occupied. It has an offset to the previous block, so you can jump back to the previous if you wanted to.

```
0x0000000000000015 0xFFFFFFFFFFFFFFFF 0x4142434400
0x0000000007FFFFEB 0x0000000000000015 ...
```

When you deallocate "ABCD", it will jump to the next block, check if it's free. Jump to previous block, check if it's free. Then merge the free blocks into 1 big block.

## dlloc

```cpp
//O(1)
bool MemoryPage::dolloc(void *ptr) {
    
    //Check if inbounds
    if(ptr < data || ptr - sizeof(MemoryBlock) < data || ptr >= data + size)
        return false;
    
    //Get current block
    u8 *beg = (u8*) ptr - sizeof(MemoryBlock);
    MemoryBlock *memb = (MemoryBlock*) beg;
    bool isFree = memb->prev >= 0;
    
    if(isFree)
        return false;
    
    //Free block
    memb->prev = ~memb->prev;
    
    //Get neighbors
    MemoryBlock* next = (MemoryBlock*)(beg + memb->size);
    MemoryBlock* prev = (MemoryBlock*)(beg - memb->prev));
    
    //No free blocks before or after
    if(prev->prev < 0 && next->prev < 0)
        return false;
    
    //TODO: Update right neighbor with prev size
    
    if(prev->prev >= 0 && next->prev >= 0)			//Merge prev and next; they're free
        prev->size += memb->size + next->size;
    else if(prev->prev >= 0)						//Merge with prev
        prev->size += memb->size;
    else											//Merge with next
        memb->size += next->size;
    
    return true;
}
```

## alloc

```cpp
//O(n)
void *MemoryPage::olloc(size_t len) {
    
    //Check if it can be allocated
    if(len >= size)
        return nullptr;
    
    //Get current block
    MemoryBlock *memb = (MemoryBlock*) data;
    
    //TODO: Only needs a MemoryBlock if there's left over memory
    
    len += sizeof(MemoryBlock);
    
    //Get next 
    while(memb->prev < 0 || memb->size < len)
        memb = (u8*) memb + memb->size;
    
    if(memb->size < len || memb->prev < 0)
        return nullptr;
    
    //Alloc block
    memb->prev = ~memb->prev;
    memb->size = len;
    
    //TODO: Fill in next memory block
    
    return true;
}
```

## Cleaning/allocating the first pages

```cpp
//onstartup:
MemoryPage pages[MemoryPage::maxPages];
pages[0] = MemoryPage(MemoryPage::defaultPageSize);
atexit(cleanup);
```