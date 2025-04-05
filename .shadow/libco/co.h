
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define DEFUALT_STACK_SIZE 1024*1

enum co_status {
    CO_NEW = 1, // 新创建，还未执行过
    CO_RUNNING, // 已经执行过
    CO_WAITING, // 在 co_wait 上等待
    CO_DEAD,    // 已经结束，但还未释放资源
};

typedef struct reg{
#if __x86_64__
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
#else
    uint64_t eax;
    uint64_t ebx;
    uint64_t ecx;
    uint64_t edx;
    uint64_t esi;
    uint64_t edi;
    uint64_t ebp;
    uint64_t esp;
#endif
}reg;

typedef struct co {
    const char *name;
    void (*func)(void*);
    void *arg;
    
    reg context;
    enum co_status status;
    uint8_t stack[DEFUALT_STACK_SIZE];
    uintptr_t stackBase;
    unsigned int pid;
    struct co *next;
}coNode;

struct co* co_start(const char *name, void (*func)(void *), void *arg);
void co_yield();
void co_wait(struct co *co);
