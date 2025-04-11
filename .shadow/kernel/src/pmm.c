#include <common.h>
#include <threads.h>

enum blockStatus {
    bnon = 1,
    bused,
    busing,
};

typedef struct blockLink{
    struct blockLink *next;
    size_t size;
    lock_t lock;
    enum blockStatus status;
}blockLink_t;

size_t blockSize;
size_t blockAllocateBit;
#define byteAlignment 16

blockLink_t bstart,*pbend;
size_t freeBytesRemaining;
size_t minFreeBytesRemaining;

// static void kinit(){
//     size_t heapStartAddr = (size_t) heap.start;
//     size_t heapEndAddr = (size_t) heap.end;

//     // set two constant
//     blockSize = sizeof(blockLink_t);
//     blockAllocateBit = ((size_t) 1) << (sizeof(size_t)*8-1);

//     // start align upward, end align downward
//     heapStartAddr += byteAlignment - 1;
//     heapStartAddr &= ~(byteAlignment - 1);

//     heapEndAddr &= ~(byteAlignment - 1);
//     heapEndAddr -= blockSize;

//     // bstart ----> first heap block ----> pbend
//     bstart.next = (blockLink_t *)heapStartAddr;
//     bstart.size = 0;
//     lock_init(&bstart.lock);  // assume start.lock is list's overall lock

//     pbend->next = NULL;
//     pbend->size = 0;

//     // set firstblock, and size include block struct
//     blockLink_t *bFirstBlock = (void *)heapStartAddr;
//     bFirstBlock->next = pbend;
//     bFirstBlock->size = heapEndAddr - heapStartAddr;

//     // set heapblock remain size
//     freeBytesRemaining = bFirstBlock->size;
//     minFreeBytesRemaining = freeBytesRemaining;
//     return;
// }

static void *kalloc(size_t size) {
    
    // // align size
    // assert(size==0);
    // size += blockSize;      // need a blocksize
    // size_t trueSize = 1;
    // while(trueSize < size)
    //     trueSize <<= 1;
    // size = trueSize;
    

    // // get list lock to find suit block, suit= enough/nowait/
    // blockLink_t *pblock = &bstart;
    // lock(&bstart.lock);
    // while (pblock != pbend){
    //     pblock = pblock->next;
    //     if(pblock->status == bnon && pblock->size >= size)
    //         break;
    // }
    // if(pblock == pbend){
    //     while (pblock != pbend){
    //         pblock = pblock->next;
    //         if(pblock->size >= size)
    //             break;
    //     }
    // }
    // assert(pblock == pbend);
    // pblock->status = busing;
    // unlock(&bstart.lock);

    // lock(&pblock->lock);


    return NULL;
}

static void kfree(void *ptr) {
    
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
}

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    .alloc = kalloc,
    .free  = kfree,
};
