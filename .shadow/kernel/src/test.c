#include <common.h>

void test0(void)
{
    printf("start\n");
	void *add1 = pmm->alloc(1020);
	if (add1 == NULL)
	{
		printf("add is NULL");
		assert(0);
	}
	else
		printf("add1: %p", add1);

	int *add1_int=(int *)add1;
	*add1_int=9876;
	*(add1_int+(1020-4)/4)=114514;
	void *add2 = pmm->alloc(20);
	if (add2 == NULL)
	{
		printf("add2 is NULL");
		assert(0);
	}
	else
		printf("add2: %p", add2);
	
	void *add3 = pmm->alloc(512);
	if (add3 == NULL)
	{
		printf("add3 is NULL");
		assert(0);
	}
	else
		printf("add3: %p", add3);
	int *add3_int=(int *)add3;
	*add3_int=543210;
	*(add3_int+(512-4)/4)=114514;
	pmm->free(add2);
    printf("cpu:%d\n",cpu_current());

    // printf("add1:%d\n",*add1_int);
	assert(*add1_int==9876);
	assert(*(add1_int+(1020-4)/4)==114514);
	assert(*add3_int==543210);
	assert(*(add3_int+(512-4)/4)==114514);
	pmm->free(add1);
	assert(*add3_int==543210);
	assert(*(add3_int+(512-4)/4)==114514);
	pmm->free(add3);


	    // // 分配 32 字节
		// void *add32 = pmm->alloc(32);
		// assert(add32 != NULL);
		// // printf("add32: %p\n", add32);
		// int *add32_int = (int *)add32;
		// *add32_int = 123;
		// *(add32_int + 6) = 456; // 假设使用满 32B
	
		// // 分配 64 字节
		// void *add64 = pmm->alloc(64);
		// assert(add64 != NULL);
		// // printf("add64: %p\n", add64);
		// int *add64_int = (int *)add64;
		// *add64_int = 789;
		// *(add64_int + 15) = 101112;
	
		// // 分配 128 字节
		// void *add128 = pmm->alloc(128);
		// assert(add128 != NULL);
		// // printf("add128: %p\n", add128);
		// int *add128_int = (int *)add128;
		// *add128_int = 2024;
		// *(add128_int + 31) = 56789;
	
		// // 分配 512 字节
		// void *add512 = pmm->alloc(512);
		// assert(add512 != NULL);
		// // printf("add512: %p\n", add512);
		// int *add512_int = (int *)add512;
		// *add512_int = 314159;
		// *(add512_int + 127) = 271828;
	
		// printf("cpu:%d\n",cpu_current());
	
		// // 验证数据
		// assert(*add32_int == 123);
		// assert(*(add32_int + 6) == 456);
	
		// assert(*add64_int == 789);
		// assert(*(add64_int + 15) == 101112);
	
		// assert(*add128_int == 2024);
		// assert(*(add128_int + 31) == 56789);
	
		// assert(*add512_int == 314159);
		// assert(*(add512_int + 127) == 271828);
	
		// // 释放
		// pmm->free(add32);
		// pmm->free(add64);
		// pmm->free(add128);
	
		// // 确保 512 内容仍然可访问
		// assert(*add512_int == 314159);
		// assert(*(add512_int + 127) == 271828);
		// pmm->free(add512);
	
		// printf("test0: slab allocator passed\n");

}

void test1(void)//page test
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
	void *add1 = pmm->alloc(4096);
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
