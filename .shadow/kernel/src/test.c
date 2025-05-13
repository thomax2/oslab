#include <common.h>

void test0(void)
{
    // printf("start\n");
	// void *add1 = pmm->alloc(1020);
	// if (add1 == NULL)
	// {
	// 	printf("add is NULL");
	// 	assert(0);
	// }
	// else
	// 	printf("add1: %p", add1);

	// int *add1_int=(int *)add1;
	// *add1_int=9876;
	// *(add1_int+(1020-4)/4)=114514;
	// void *add2 = pmm->alloc(20);
	// if (add2 == NULL)
	// {
	// 	printf("add2 is NULL");
	// 	assert(0);
	// }
	// else
	// 	printf("add2: %p", add2);
	
	// void *add3 = pmm->alloc(512);
	// if (add3 == NULL)
	// {
	// 	printf("add3 is NULL");
	// 	assert(0);
	// }
	// else
	// 	printf("add3: %p", add3);
	// int *add3_int=(int *)add3;
	// *add3_int=543210;
	// *(add3_int+(512-4)/4)=114514;
	// pmm->free(add2);
    // printf("cpu:%d\n",cpu_current());
	// printf("free: add2\n");
    // // printf("add1:%d\n",*add1_int);
	// assert(*add1_int==9876);
	// assert(*(add1_int+(1020-4)/4)==114514);
	// assert(*add3_int==543210);
	// assert(*(add3_int+(512-4)/4)==114514);
	// pmm->free(add1);
	// printf("free: add1\n");
	// assert(*add3_int==543210);
	// assert(*(add3_int+(512-4)/4)==114514);
	// pmm->free(add3);
	// printf("free: add3\n");


	    // 分配 32 字节
		void *add32 = pmm->alloc(32);
		assert(add32 != NULL);
		// printf("add32: %p\n", add32);
		int *add32_int = (int *)add32;
		*add32_int = 123;
		*(add32_int + 6) = 456; // 假设使用满 32B
	
		// 分配 64 字节
		void *add64 = pmm->alloc(64);
		assert(add64 != NULL);
		// printf("add64: %p\n", add64);
		int *add64_int = (int *)add64;
		*add64_int = 789;
		*(add64_int + 15) = 101112;
	
		// 分配 128 字节
		void *add128 = pmm->alloc(128);
		assert(add128 != NULL);
		// printf("add128: %p\n", add128);
		int *add128_int = (int *)add128;
		*add128_int = 2024;
		*(add128_int + 31) = 56789;
	
		// 分配 512 字节
		void *add512 = pmm->alloc(512);
		assert(add512 != NULL);
		// printf("add512: %p\n", add512);
		int *add512_int = (int *)add512;
		*add512_int = 314159;
		*(add512_int + 127) = 271828;
	
		printf("cpu:%d\n",cpu_current());
	
		// 验证数据
		assert(*add32_int == 123);
		assert(*(add32_int + 6) == 456);
	
		assert(*add64_int == 789);
		assert(*(add64_int + 15) == 101112);
	
		assert(*add128_int == 2024);
		assert(*(add128_int + 31) == 56789);
	
		assert(*add512_int == 314159);
		assert(*(add512_int + 127) == 271828);
	
		// 释放
		pmm->free(add32);
		pmm->free(add64);
		pmm->free(add128);
	
		// 确保 512 内容仍然可访问
		assert(*add512_int == 314159);
		assert(*(add512_int + 127) == 271828);
		pmm->free(add512);
	
		printf("test0: slab allocator passed\n");

}

void test_repeated_alloc(void) {
    const size_t ALLOC_SIZE = 64; // 测试 64 字节分配
    const int MAX_ALLOC_TIMES = 63; // 分配 128 次
    void *alloc_ptrs[MAX_ALLOC_TIMES]; // 存储分配地址

    // ==== 阶段 1：连续分配并填充数据 ====
    for (int i = 0; i < MAX_ALLOC_TIMES; i++) {
        // 分配内存
        alloc_ptrs[i] = pmm->alloc(ALLOC_SIZE);
        assert(alloc_ptrs[i] != NULL);

        // 填充模式：前 4 字节写入序号 i，最后 4 字节写入 ~i（按位取反）
        int *ptr = (int *)alloc_ptrs[i];
        *ptr = i; // 头部标记
        *(int *)((char *)ptr + ALLOC_SIZE - sizeof(int)) = ~i; // 尾部标记
		// printf("iiii:%d\n",i);
    }
	printf("alloc sucess\n");

    // ==== 阶段 2：验证数据完整性 ====
    for (int i = 0; i < MAX_ALLOC_TIMES; i++) {
        int *ptr = (int *)alloc_ptrs[i];
        assert(*ptr == i); // 检查头部
        assert(*(int *)((char *)ptr + ALLOC_SIZE - sizeof(int)) == ~i); // 检查尾部
    }
	printf("verify sucess\n");

    // ==== 阶段 3：交替释放并重新分配 ====
    for (int i = 0; i < MAX_ALLOC_TIMES; i += 2) {
        // 释放偶数序号块
        pmm->free(alloc_ptrs[i]);
        alloc_ptrs[i] = NULL;

		printf("realse even %d\n",cpu_current());
        // 重新分配并验证新块独立性
        void *new_ptr = pmm->alloc(ALLOC_SIZE);
        assert(new_ptr != NULL);

		printf("new assign %d\n",cpu_current());
        // 新块头部应为未初始化值（若分配器不自动清零）
        // 写入新数据并验证
        *(int *)new_ptr = 0xDEADBEEF;
        assert(*(int *)new_ptr == 0xDEADBEEF);

        // 暂存新指针，稍后统一释放
        alloc_ptrs[i] = new_ptr;
    }

    // ==== 阶段 4：释放所有内存 ====
    for (int i = 0; i < MAX_ALLOC_TIMES; i++) {
        if (alloc_ptrs[i] != NULL) {
            pmm->free(alloc_ptrs[i]);
        }
    }

    printf("test_repeated_alloc: 64-byte repeated allocation passed\n");
}

void test1(void)//page test
{
	void *add = pmm->alloc(4096*2);
	if (add == NULL)
	{
		printf("add is NULL");
		assert(0);
	}
	else
		printf("add: %p\n", add);
	char *add_char=(char *)add;
	*add_char='a';
	*(add_char+4095)='b';
	void *add1 = pmm->alloc(4096*2);
	if (add1 == NULL)
	{
		printf("add1 is NULL");
		assert(0);
	}
	else
		printf("add1: %p\n", add1);

	// for(int i=0;i<=5;++i)
	// {
	// 	void *add = pmm->alloc(4096);
	// 	if (add == NULL)
	// 	{
	// 		printf("add is NULL");
	// 		assert(0);
	// 	}
	// 	else
	// 		printf("add: %p\n", add);
	// }

	char *add_char1=(char *)add1;
	*add_char1='c';
	*(add_char1+4095)='d';
	assert(*add_char=='a');
	assert(*(add_char+4095)=='b');
	pmm->free(add);
	assert(*add_char1=='c');
	assert(*(add_char1+4095)=='d');
	pmm->free(add1);
}

void test2(void) //混合的内存申请
{
	void *add = pmm->alloc(4096);
	if (add == NULL)
	{
		printf("add is NULL");
		assert(0);
	}
	else
		printf("add: %p\n", add);
	char *add_char=(char *)add;
	*add_char='a';
	*(add_char+4095)='b';
	void *add1 = pmm->alloc(1024);
	if (add1 == NULL)
	{
		printf("add1 is NULL");
		assert(0);
	}
	else
		printf("add1: %p\n", add1);
	char *add_char1=(char *)add1;
	*add_char1='c';
	*(add_char1+1023)='d';
	assert(*add_char=='a');
	assert(*(add_char+4095)=='b');
	pmm->free(add);
	assert(*add_char1=='c');
	assert(*(add_char1+1023)=='d');
	void *add2 = pmm->alloc(45);
	if (add2 == NULL)
	{
		printf("add2 is NULL");
		assert(0);
	}
	else
		printf("add2: %p\n", add2);
	char *add_char2=(char *)add2;
	*add_char2='e';
	*(add_char2+44)='f';
	pmm->free(add1);
	assert(*add_char2=='e');
	assert(*(add_char2+44)=='f');
	add = pmm->alloc(4096);
	if (add == NULL)
	{
		printf("add is NULL");
		assert(0);
	}
	else
		printf("add: %p\n", add);
	add_char=(char *)add;
	*add_char='g';
	*(add_char+4095)='h';
	pmm->free(add2);
	assert(*add_char=='g');
	assert(*(add_char+4095)=='h');
	pmm->free(add);
}
