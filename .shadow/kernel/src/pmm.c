#include <common.h>
// #include <threads.h>

enum blockStatus {
    bnon = 1,
    bused,
};

typedef struct blockLink{
    struct blockLink *next;
    size_t size;
}blockLink_t;

lock_t linkLock;

size_t blockSize;
size_t blockAllocateBit;
#define byteAlignment 16

blockLink_t bstart,*pbend;
size_t freeBytesRemaining;
size_t maxFreeBytesRemaining;

static void kinit(void){
    size_t heapStartAddr = (size_t) heap.start;
    size_t heapEndAddr = (size_t) heap.end;

    // set two constant
    blockSize = sizeof(blockLink_t);
    blockAllocateBit = ((size_t) 1) << (sizeof(size_t)*8-1);

    // start align upward, end align downward
    heapStartAddr += byteAlignment - 1;
    heapStartAddr &= ~(byteAlignment - 1);

    heapEndAddr &= ~(byteAlignment - 1);
    heapEndAddr -= blockSize;

    // bstart ----> first heap block ----> pbend
    bstart.next = (blockLink_t *)heapStartAddr;
    bstart.size = 0;
    // lock_init(&bstart.lock);  // assume start.lock is list's overall lock
    lock_init(&linkLock);

    pbend->next = NULL;
    pbend->size = 0;

    // set firstblock, and size not include block struct
    blockLink_t *bFirstBlock = (void *)heapStartAddr;
    bFirstBlock->next = pbend;
    bFirstBlock->size = heapEndAddr - heapStartAddr - blockSize;

    // set heapblock remain size
    freeBytesRemaining = bFirstBlock->size;
    maxFreeBytesRemaining = freeBytesRemaining;
    return;
}

size_t addr_valid(blockLink_t *pblock, size_t size, size_t addrMod){
    size_t startAddr = (size_t)pblock + blockSize;
    if(startAddr%addrMod == 0)
        return startAddr;
    startAddr += blockSize;
    size_t validAddr = startAddr;
    validAddr += addrMod - (validAddr%addrMod);
    if (validAddr + size <= (size_t)pblock + pblock->size)
        return validAddr;
    else
        return 0;
}

static void *kalloc(size_t size) {
    // printf("%d\n",size);
    // align size
    assert(size != 0);
    size_t addrMod = 1;
    while(addrMod < size)
        addrMod <<= 1;

    // get list lock to find suit block, suit= enough/nowait/
    blockLink_t *pblock = bstart.next;
    size_t addr=0;
    lock(&linkLock);
    while (pblock != pbend)
    {
        if(!(pblock->size & blockAllocateBit) && pblock->size >= size)    // block not allocate
            if((addr = addr_valid(pblock, size, addrMod)) != 0) // size enough
                break;
        pblock = pblock->next;
    }
    for (blockLink_t *pblock = &bstart; pblock; pblock = pblock->next)
    {
        printf("addr:%p\n",pblock);
        printf("size:%d\n",pblock->size);
        printf("-----\n");
    }
    printf("******\n");
    assert(addr != 0);
    
    if(addr - blockSize == (size_t)pblock)
    {
        blockLink_t *bNextBlock = (void *)(addr + size);
        bNextBlock->next = pblock->next;
        bNextBlock->size = pblock->size - size - blockSize;

        pblock->next = bNextBlock;
        pblock->size = size;
        pblock->size |= (blockAllocateBit);
    }
    else
    {
        blockLink_t *bNextBlock = (void *)(addr + size);
        blockLink_t *bNowBlock = (void *)(addr - blockSize);
        bNextBlock->next = pblock->next;
        bNowBlock->next = bNextBlock;
        pblock->next = bNowBlock;

        bNextBlock->size = pblock->size - (addr - (size_t)pblock) - size - blockSize;
        bNowBlock->size = size;
        bNowBlock->size |= blockAllocateBit;
        pblock->size = addr - (size_t)pblock - blockSize;
    }
    unlock(&linkLock);

    return (void *)addr;
}

static void kfree(void *ptr) {
    blockLink_t *pblock = (void *)((size_t)ptr - blockSize);
    blockLink_t *ppreBlock;
    blockLink_t *pafterBlock = pblock->next;
    lock(&linkLock);
    //find pre block
    for(ppreBlock = &bstart; ppreBlock->next != pblock; ppreBlock = ppreBlock->next);
    
    // merge pre block
    if(!(ppreBlock->size & blockAllocateBit))
    {
        ppreBlock->size += pblock->size + blockSize;
        ppreBlock->next = pblock->next;
        pblock = ppreBlock;
    }

    // merge after block
    if(!(pafterBlock->size & blockAllocateBit))
    {
        pblock->size += pafterBlock->size + blockSize;
        pblock->next = pafterBlock->next;
    }
    unlock(&linkLock);
    return;
}

static void pmm_init() {
    uintptr_t pmsize = (
        (uintptr_t)heap.end
        - (uintptr_t)heap.start
    );

    printf(
        "Got %d MiB heap: [%p, %p)\n",
        pmsize >> 20, heap.start, heap.end
    );

    kinit();

}

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    .alloc = kalloc,
    .free  = kfree,
};
