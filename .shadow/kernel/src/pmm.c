#include <common.h>
// #include <threads.h>

#define PAGE_SIZE 4096

// 4KB
#define SLAB_SIZE PAGE_SIZE
// 64B ~ 2KB
#define SLAB_NUM 6 

// 4MB
#define BUDDY_SIZE 4*1024*1024
// 4KB ~ 1MB
#define BUDDY_NUM 9

#define CPU_NUM 4


typedef struct slab_page
{
    uintptr_t start;
    uintptr_t head_free;
    struct slab_page *next_page;
    size_t size;
    size_t remain_unit_num;
}slab_page;

struct Manager_slab_area{
    slab_page *start[SLAB_NUM];
    slab_page *pos[SLAB_NUM];
}manager_slab_area[CPU_NUM];


struct Slab_info{    
    slab_page page[SLAB_NUM];
}slab_info[CPU_NUM];

typedef struct buddy_zone
{
    lock_t buddy_lk;
    uintptr_t start;
    uintptr_t head_free;
    size_t size;
    size_t remain_unit_num;
}buddy_zone;

struct Buddy_info{
    buddy_zone zone[BUDDY_NUM];
}buddy_info;

size_t buddy_start;

// typedef struct {
//     unsigned int bits[BITMAP_SIZE / (sizeof(unsigned int) * 8)]; // 根据unsigned int的大小来计算数组大小
// } Bitmap;

void *buddy_alloc(size_t size);
void *alloc_in_page(slab_page *page_ptr, int cpu, int page_num);
void *slab_alloc(size_t size);

static void kinit(void){
    size_t heapStartAddr = (size_t) heap.start;
    size_t heapEndAddr = (size_t) heap.end;
    
    for (size_t i = 0; i < CPU_NUM; i++){
        for (size_t j = 0; j < SLAB_NUM; j++){
            manager_slab_area[i].start[j] = (slab_page *)(heapEndAddr - 256 * SLAB_NUM * (CPU_NUM - 1 - i) - 256 * (SLAB_NUM - j));
            manager_slab_area[i].pos[j] = manager_slab_area[i].start[j];
        }
    }
    for(size_t cpu_num = 0; cpu_num < CPU_NUM; cpu_num++)
    {
        for(size_t i=0;i<SLAB_NUM;i++)
        {
            slab_info[cpu_num].page[i].size = 1<<(i+6);
            slab_info[cpu_num].page[i].start = (uintptr_t)(heapStartAddr + i*SLAB_SIZE + cpu_num * SLAB_SIZE*SLAB_NUM);
            slab_info[cpu_num].page[i].remain_unit_num = SLAB_SIZE/(slab_info[cpu_num].page[i].size);
            slab_info[cpu_num].page[i].head_free = slab_info[cpu_num].page[i].start;
            slab_info[cpu_num].page[i].next_page = NULL;
            
            char *bpos = (char *)(slab_info[cpu_num].page[i].head_free);
            for(int k=0; k < slab_info[cpu_num].page[i].remain_unit_num - 1;k++)
            {
                *(uintptr_t *)bpos = (uintptr_t)(bpos + slab_info[cpu_num].page[i].size);
                bpos += slab_info[cpu_num].page[i].size;
            }
            *(uintptr_t *)bpos = (uintptr_t)NULL;
        }
    }

    // buddy_start = heapStartAddr + CPU_NUM*SLAB_NUM*SLAB_SIZE;
    buddy_start = heapStartAddr + 1*1024*1024;
    for (size_t i = 0; i < BUDDY_NUM; i++)
    {
        buddy_info.zone[i].start = buddy_start + i * BUDDY_SIZE;
        buddy_info.zone[i].head_free = buddy_info.zone[i].start;
        buddy_info.zone[i].size = 1<<(i+12);
        buddy_info.zone[i].remain_unit_num = (BUDDY_SIZE)/(buddy_info.zone[i].size);

        char *bpos = (char *)(buddy_info.zone[i].head_free);
        for(int k=0; k < buddy_info.zone[i].remain_unit_num - 1; k++)
        {
            // (uintptr_t)((size_t)block->head_free + k)
            *(uintptr_t *)bpos = (uintptr_t)(bpos + buddy_info.zone[i].size);
            bpos += buddy_info.zone[i].size;
        }
        *(uintptr_t *)bpos = (uintptr_t)NULL;
    }
    
    for (size_t i = 0; i < BUDDY_NUM; i++)
        lock_init(&(buddy_info.zone[i].buddy_lk));

    return;
}

size_t align_size(size_t size)
{
    if((size & (size - 1)) == 0) {
        if(size < 64)
            size = 64;
        return size;
    }
    size_t power = 1;
    while (power < size)
    {
        power <<= 1;
    }
    if(power < 64)
        power = 64;
    return power;
}

size_t get_index(size_t size)
{
    int pow = 0;
    while (size > 1){
        pow += 1;
        size /= 2;
    }
    return pow;
}

void *alloc_in_page(slab_page *page_ptr, int cpu, int page_num) {
    void *addr=NULL;

    page_ptr->remain_unit_num --;
    addr = (void *)page_ptr->head_free;
    page_ptr->head_free = *(uintptr_t *)addr;
    assert(addr != NULL);
    return addr;
}

void *slab_alloc(size_t size) {
    int cpu = cpu_current();
    int page_num = get_index(size >> 6);

    if(slab_info[cpu].page[page_num].remain_unit_num >= 1){
        return alloc_in_page(&(slab_info[cpu].page[page_num]), cpu, page_num);
    }
    else if( slab_info[cpu].page[page_num].remain_unit_num == 0 ) {

        slab_page *per_page_ptr = &(slab_info[cpu].page[page_num]);
        slab_page *page_ptr = per_page_ptr->next_page;


        while (page_ptr != NULL)
        {
            if(page_ptr->remain_unit_num > 0){
                return alloc_in_page(page_ptr, cpu, page_num);
            }
            per_page_ptr = page_ptr;
            page_ptr = page_ptr->next_page;
        }
        
        if(page_ptr == NULL)
        {

            slab_page *next_page_ptr = (slab_page *)manager_slab_area[cpu].pos[page_num];
            void *new_page = buddy_alloc(SLAB_SIZE);
            manager_slab_area[cpu].pos[page_num] += sizeof(slab_page);

            next_page_ptr->next_page = NULL;
            next_page_ptr->remain_unit_num = (SLAB_SIZE)/(slab_info[cpu].page[page_num].size) - 1;
            next_page_ptr->size = slab_info[cpu].page[page_num].size;
            next_page_ptr->start = (uintptr_t)new_page;
            next_page_ptr->head_free = (uintptr_t)((size_t)next_page_ptr->start + next_page_ptr->size);

            char *bpos = (char *)next_page_ptr->head_free;
            for(int k=0; k < next_page_ptr->remain_unit_num - 1; k++)
            {
                *(uintptr_t *)bpos = (uintptr_t)(bpos + next_page_ptr->size);
                bpos += next_page_ptr->size;
            }
            *(uintptr_t *)bpos = (uintptr_t)NULL;
            per_page_ptr -> next_page = next_page_ptr;
            page_ptr = next_page_ptr;
        }
        return alloc_in_page(page_ptr, cpu, page_num);
    }
    else{
        assert(0);
    }
    return NULL;
}

void *buddy_alloc(size_t size){
    int zone_num = get_index(size >> 12);
    void *addr = NULL;
    lock(&(buddy_info.zone[zone_num].buddy_lk));
    buddy_info.zone[zone_num].remain_unit_num --;
    addr = (void *)buddy_info.zone[zone_num].head_free;
    buddy_info.zone[zone_num].head_free = *(uintptr_t *)addr;
    unlock(&(buddy_info.zone[zone_num].buddy_lk));
    assert(addr != NULL);
    return addr;
}

static void *kalloc(size_t size) {
    // align size
    assert(size != 0);
    void *addr = NULL;

    size = align_size(size);
    printf("pmm size:%d\n",size);
    assert(size >= 64);
    if(size < SLAB_SIZE)
        addr = slab_alloc(size);
    else if( size < BUDDY_SIZE )
        addr = buddy_alloc(size);
    else if( size < 16*1024*1024)
        {}// addr = huge_alloc(size);
    else
        assert(0);
    return addr;
}

static void kfree(void *ptr) {
    // in slab
    if((size_t)ptr < (size_t)buddy_start){
        int cpu = cpu_current();
        slab_page *page_ptr = NULL;
        for (size_t j = 1; j < SLAB_NUM; j++){
            if((size_t)slab_info[cpu].page[j].start > (size_t)ptr)
            {
                page_ptr = &(slab_info[cpu].page[j-1]);
                break;
            }
        }
        if(page_ptr == NULL)
            page_ptr = &(slab_info[cpu].page[SLAB_NUM-1]);
        page_ptr->remain_unit_num ++;
        *(uintptr_t *)ptr = page_ptr->head_free;
        page_ptr->head_free = (uintptr_t)ptr;
        assert(page_ptr->head_free != (uintptr_t)NULL);
    }
    else { // in buddy
        buddy_zone *zone_ptr = NULL;
        size_t i;
        for(i = 0; i < BUDDY_NUM; i++){
            if((size_t)buddy_info.zone[i].start > (size_t)ptr) {
                zone_ptr = &(buddy_info.zone[i-1]);
                break;
            }
        }
        if(i == 1 && ((char *)ptr - (char *)zone_ptr->start) % PAGE_SIZE != 0){ // 4KB buddy assign to slab
            int cpu = cpu_current();
            slab_page *page_ptr;
            size_t page_start = (size_t)((char *)ptr - ((char *)ptr - (char *)zone_ptr->start) % PAGE_SIZE);
            for (size_t i = 0; i < SLAB_NUM; i++)
            {
                page_ptr = &(slab_info[cpu].page[i]);
                while (page_ptr != NULL)
                {
                    if((size_t)page_ptr->start == page_start)
                        goto found;
                    page_ptr = page_ptr->next_page;
                }
            }
            found:
            assert((size_t)page_ptr->start == page_start);
            lock(&(zone_ptr->buddy_lk));
            page_ptr->remain_unit_num ++;
            *(uintptr_t *)ptr = page_ptr->head_free;
            page_ptr->head_free = (uintptr_t)ptr;
            unlock(&(zone_ptr->buddy_lk));
            return;
        }

        if(zone_ptr == NULL)
        {
            zone_ptr = &(buddy_info.zone[BUDDY_NUM-1]);
        }

        lock(&(zone_ptr->buddy_lk));
        zone_ptr->remain_unit_num ++;
        *(uintptr_t *)ptr = zone_ptr->head_free;
        zone_ptr->head_free = (uintptr_t)ptr;
        unlock(&(zone_ptr->buddy_lk));
    }
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

    printf("cpu_count: %d\n",cpu_count());
    kinit();
    
}

static void *kalloc_irq(size_t size)
{
    int i = ienabled();
    iset(false);
    void *ret = kalloc(size);
    if(i) iset(true);
    return ret;
}


static void kfree_irq(void *ptr)
{
    int i = ienabled();
    iset(false);
    kfree(ptr);
    if(i) iset(true);
    return;
}

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    // .alloc = kalloc,
    // .free  = kfree,
    .alloc = kalloc_irq,
    .free  = kfree_irq,
};
