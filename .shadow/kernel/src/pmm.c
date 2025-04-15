#include <common.h>
// #include <threads.h>

enum blockStatus {
    bnon = 1,
    bused,
};

typedef struct blockLink{
    struct blockLink *next;
    size_t size;
    lock_t lk;
}blockLink_t;

// lock_t linkLock;

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
    lock_init(&bstart.lk);  // assume start.lock is list's overall lock
    // lock_init(&bstart);

    pbend->next = NULL;
    pbend->size = 0;
    lock_init(&pbend->lk);

    // set firstblock, and size not include block struct
    blockLink_t *bFirstBlock = (void *)heapStartAddr;
    bFirstBlock->next = pbend;
    bFirstBlock->size = heapEndAddr - heapStartAddr - blockSize;
    lock_init(&bFirstBlock->lk);

    // set heapblock remain size
    freeBytesRemaining = bFirstBlock->size;
    maxFreeBytesRemaining = freeBytesRemaining;

    return;
}

size_t addr_valid(blockLink_t *pblock, size_t size, size_t addrMod){
    size_t startAddr = (size_t)pblock + blockSize;
    // printf("starAddr %x\n", startAddr);
    if(startAddr%addrMod == 0)
        return startAddr;
    startAddr += blockSize;
    size_t validAddr = startAddr+1;
    // printf("validAddr %x\n", validAddr);
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
    size_t addr=0;

    blockLink_t *preblock = &bstart;
    lock(&preblock->lk);
    lock(&preblock->next->lk);
    blockLink_t *pblock =preblock->next;

    // get usable block lock and pre block lock 
    while (pblock != pbend)
    {
        if(pblock->size & blockAllocateBit) // get free block but alloced
        {
            unlock(&pblock->lk);
            unlock(&preblock->lk);
            preblock = &bstart;
            lock(&preblock->lk);
            lock(&preblock->next->lk);
            pblock = preblock->next;
        }
        else                                // get free block and free
        {
            size_t trueSize = pblock->size & ~blockAllocateBit;
            if(trueSize >= size)
                if((addr = addr_valid(pblock, size, addrMod)) != 0)
                    break;
            
            lock(&pblock->next->lk);
            unlock(&preblock->lk);
            preblock = pblock;
            pblock = pblock->next;
        }
    }

    assert(pblock != pbend);
    assert(addr != 0);

    // printf("cpu:%d\n",cpu_current());    

    size_t trueSize = (pblock->size) & ~(blockAllocateBit);    


    lock(&pblock->next->lk);
    lock_t *lk3 = &pblock->next->lk;
    lock_t *lk2 = &pblock->lk;
    lock_t *lk1 = &preblock->lk;

    if(addr - blockSize == (size_t)pblock)
    {   
        // delete block, insert new free block
        if(addr + size < (size_t)pblock + trueSize)
        {
            blockLink_t *bNewBlock = (void *)(addr + size);
            lock_init(&bNewBlock->lk);
            bNewBlock->next = pblock->next;
            preblock->next = bNewBlock;
            bNewBlock->size = trueSize - size - blockSize;
            bNewBlock->size &= ~(blockAllocateBit);

            pblock->next = (struct blockLink *)0x55555555;       // specify allocated block next=0x55555555
            pblock->size = size;
            pblock->size |= (blockAllocateBit);
            unlock(&bNewBlock->next->lk);

        }
        else    // delete block
        {
            preblock->next = pblock->next;
            pblock->next = (struct blockLink *)0x55555555;
            pblock->size |= (blockAllocateBit);
        }
    }
    else
    {
        // insert new free block
        if(addr + size < (size_t)pblock + trueSize)
        {
            blockLink_t *bNewBlock = (void *)(addr + size);
            blockLink_t *bNowBlock = (void *)(addr - blockSize);
            lock_init(&bNewBlock->lk);
            lock_init(&bNowBlock->lk);
            bNewBlock->next = pblock->next;
            bNowBlock->next = (struct blockLink *)0x55555555;
            pblock->next = bNewBlock ;
    
            bNowBlock->size = (size_t)(bNewBlock) - addr;
            bNowBlock->size |= blockAllocateBit;
            pblock->size = (size_t)bNowBlock - (size_t)pblock - blockSize;
            pblock->size &= ~(blockAllocateBit);
            bNewBlock->size = trueSize - pblock->size - (bNowBlock->size & ~(blockAllocateBit)) - 2*blockSize;
            bNewBlock->size &= ~(blockAllocateBit);
            
        }
        else // non delete/insert, just change size
        {
            blockLink_t *bNewBlock = (void *)(addr - blockSize);
            lock_init(&bNewBlock->lk);
            bNewBlock->next = (struct blockLink *)0x55555555;

            pblock->size = (size_t)bNewBlock - (size_t)pblock - blockSize;
            pblock->size &= ~(blockAllocateBit);
            bNewBlock->size = trueSize - pblock->size - blockSize;
            bNewBlock->size |= (blockAllocateBit);    
            
        }
    }

    unlock(lk3);
    unlock(lk2);
    unlock(lk1);
    return (void *)addr;
}

static void kfree(void *ptr) {
    blockLink_t *pblock = (void *)((size_t)ptr - blockSize);
    blockLink_t *preBlock;
    blockLink_t *pafterBlock;

    lock(&bstart.lk);
    preBlock = &bstart;
    lock(&preBlock->next->lk);
    pafterBlock = preBlock->next;

    //find pre & after block
    while ((size_t)pafterBlock < (size_t) pblock)
    {
        lock(&pafterBlock->next->lk);
        pafterBlock = pafterBlock->next;
        unlock(&preBlock->lk);
        preBlock = pafterBlock;
    }
    assert(preBlock->next == pafterBlock);
    
    preBlock->next = pblock;
    pblock->next =pafterBlock;

    lock_t *lk1 = &preBlock->lk;
    lock_t *lk2 = &pafterBlock->lk;
    // size_t trueSize = pblock->size & ~blockAllocateBit;
    // merge pre block
    if( (size_t)pblock - ((size_t)preBlock + preBlock->size + blockSize) < blockSize )
    {
        preBlock->size = (size_t)(pblock) - (size_t)preBlock + pblock->size;
        preBlock->next = pafterBlock;
        pblock = preBlock;
        pblock->size &= ~(blockAllocateBit);
    }

    // merge after block
    if(!(pafterBlock->size & blockAllocateBit))
    {
        pblock->size = (size_t)(pafterBlock) - (size_t)pblock + pafterBlock->size;
        pblock->size &= ~(blockAllocateBit);
        pblock->next = pafterBlock->next;
    }

    unlock(lk2);
    unlock(lk1);
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
