/*
1. malloc & free complete but free too difficult
2. co_start
3. main initial co 
4. yield = save context + change context + 
*/
#include "co.h"
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define DEFUALT_STACK_SIZE 1024*32

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
    struct co *next;
}coNode;

coNode coMain = {
    .name = "main",
    .next = NULL,
    .status = CO_RUNNING
};

coNode *currentCo = &coMain;
coNode *oldCurrentCo;
coNode *newCurrentCo;

void insert_co(coNode *coNew)
{
    coNode *listEnd= &coMain;
    while (listEnd->next != NULL)
        listEnd = listEnd->next;
    listEnd->next = coNew;
    coNew->next = NULL;
    return;
}

void remove_co(coNode *co)
{
    coNode *preCoNode = &coMain;
    while (preCoNode->next != NULL)
    {
        if (preCoNode->next == co)
            break;
        preCoNode = preCoNode->next;
    }
    preCoNode->next = co->next;
}

void coroutine_wrapper() {
    currentCo->status = CO_RUNNING;

    currentCo->func(currentCo->arg);
    currentCo->status = CO_DEAD;
    co_yield();
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
    coNode *coNew = (coNode *)malloc(sizeof(coNode));
    coNew->name = name;
    coNew->func = func;
    coNew->arg = arg;
    coNew->next = NULL;
    coNew->status = CO_NEW;
    coNew->stackBase = (((uintptr_t)coNew->stack + DEFUALT_STACK_SIZE - 1) & (~(0xF)) )- 8;
    insert_co(coNew);
    return coNew;
}

void co_wait(struct co *co) {
    currentCo->status = CO_WAITING;
    while (1)
    {
        if(co->status!=CO_DEAD)
            co_yield();
        else
            break;
    }
    
    remove_co(co);
    free(co);
    currentCo->status = CO_RUNNING;
}

int get_coNum(void)
{
    int num = 1;
    coNode *list= &coMain;
    while (list->next != NULL)
    {
        if(list->next->status != CO_DEAD)
            num++;
        list = list->next;
    }
    return num;
}

void co_yield() {
    int flag = 0;
    int coNum = get_coNum();

    int chooseNum = rand()%(coNum); // [0,coNum-1]
    newCurrentCo = &coMain ;
    for (size_t i = 0; i < chooseNum; i++)
    {
        newCurrentCo = newCurrentCo -> next;
        while (newCurrentCo->status == CO_DEAD)
            newCurrentCo = newCurrentCo -> next;
    }
    
    if(newCurrentCo == currentCo)
        return;
    oldCurrentCo = currentCo;
    currentCo = newCurrentCo;

    if (currentCo->status != CO_NEW)
    {
        asm volatile(
            #if __x86_64__
            "mov %%rbx, %1;"
            "mov %%rbp, %6;"
            "mov %%r12, %12;"
            "mov %%r13, %13;"
            "mov %%r14, %14;"
            "mov %%r15, %15;"
            : "=m"(oldCurrentCo->context.rax), "=m"(oldCurrentCo->context.rbx),
              "=m"(oldCurrentCo->context.rcx), "=m"(oldCurrentCo->context.rdx),
              "=m"(oldCurrentCo->context.rsi), "=m"(oldCurrentCo->context.rdi),
              "=m"(oldCurrentCo->context.rbp), "=m"(oldCurrentCo->context.rsp),
              "=m"(oldCurrentCo->context.r8), "=m"(oldCurrentCo->context.r9),
              "=m"(oldCurrentCo->context.r10), "=m"(oldCurrentCo->context.r11),
              "=m"(oldCurrentCo->context.r12), "=m"(oldCurrentCo->context.r13),
              "=m"(oldCurrentCo->context.r14), "=m"(oldCurrentCo->context.r15)
            :
            :
            #else
            "mov %%ebx, %1;"
            "mov %%ecx, %2;"
            "mov %%edx, %3;"
            "mov %%esi, %4;"
            "mov %%edi, %5;"
            "mov %%ebp, %6;"
            : "=m"(oldCurrentCo->context.eax), "=m"(oldCurrentCo->context.ebx),
              "=m"(oldCurrentCo->context.ecx), "=m"(oldCurrentCo->context.edx),
              "=m"(oldCurrentCo->context.esi), "=m"(oldCurrentCo->context.edi),
              "=m"(oldCurrentCo->context.ebp), "=m"(oldCurrentCo->context.esp)
            :
            :
            #endif
        );
        asm volatile(
            #if __x86_64__
            "mov $0, %%r9;"
            "leaq (%%rip), %%r8;"
            "cmp $0, %%r9;"
            "jne  0f;"
            "push %%r8;"
            "mov %%rsp, %0;"

            "mov %2, %%rbx;"
            "mov %7, %%rbp;"
            "mov $1, %%r9;"
            "mov %13, %%r12;"
            "mov %14, %%r13;"
            "mov %15, %%r14;"
            "mov %16, %%r15;"
            "mov %8, %%rsp;"
            "pop %%r10;"
            "jmp *%%r10;"
            "0:\n\t"
            : "=m"(oldCurrentCo->context.rsp)
            : "m"(currentCo->context.rax), "m"(currentCo->context.rbx),
              "m"(currentCo->context.rcx), "m"(currentCo->context.rdx),
              "m"(currentCo->context.rsi), "m"(currentCo->context.rdi),
              "m"(currentCo->context.rbp), "m"(currentCo->context.rsp),
              "m"(currentCo->context.r8), "m"(currentCo->context.r9),
              "m"(currentCo->context.r10), "m"(currentCo->context.r11),
              "m"(currentCo->context.r12), "m"(currentCo->context.r13),
              "m"(currentCo->context.r14), "m"(currentCo->context.r15)
            : "memory", "r9", "r8", "rcx", "rsi"
            #else
            "mov $0, %%esi;"
            "call 1f;"
            "1: cmp $0, %%esi;"
            "jne 0f;"
            "mov %%esp, %0;"

            "mov %2, %%ebx;"
            "mov %3, %%ecx;"
            "mov $1, %%esi;"
            "mov %6, %%edi;"
            "mov %7, %%ebp;"
            "mov %8, %%esp;"
            "mov %4, %%edx;"
            "pop %%edi;"
            "jmp *%%edi;"
            "0:\n\t"
            : "=m"(oldCurrentCo->context.esp)
            : "m"(currentCo->context.eax), "m"(currentCo->context.ebx),
              "m"(currentCo->context.ecx), "m"(currentCo->context.edx),
              "m"(currentCo->context.esi), "m"(currentCo->context.edi),
              "m"(currentCo->context.ebp), "m"(currentCo->context.esp)
            : "memory"
            #endif
        );
    }
    else
    {
        asm volatile(
            #if __x86_64__
            "mov %%rbx, %1;"
            "mov %%rbp, %6;"
            "mov %%r12, %12;"
            "mov %%r13, %13;"
            "mov %%r14, %14;"
            "mov %%r15, %15;"
            : "=m"(oldCurrentCo->context.rax), "=m"(oldCurrentCo->context.rbx),
              "=m"(oldCurrentCo->context.rcx), "=m"(oldCurrentCo->context.rdx),
              "=m"(oldCurrentCo->context.rsi), "=m"(oldCurrentCo->context.rdi),
              "=m"(oldCurrentCo->context.rbp), "=m"(oldCurrentCo->context.rsp),
              "=m"(oldCurrentCo->context.r8), "=m"(oldCurrentCo->context.r9),
              "=m"(oldCurrentCo->context.r10), "=m"(oldCurrentCo->context.r11),
              "=m"(oldCurrentCo->context.r12), "=m"(oldCurrentCo->context.r13),
              "=m"(oldCurrentCo->context.r14), "=m"(oldCurrentCo->context.r15)
            :
            :
            #else
            "mov %%ebx, %1;"
            "mov %%ecx, %2;"
            "mov %%edx, %3;"
            "mov %%esi, %4;"
            "mov %%edi, %5;"
            "mov %%ebp, %6;"
            : "=m"(oldCurrentCo->context.eax), "=m"(oldCurrentCo->context.ebx),
              "=m"(oldCurrentCo->context.ecx), "=m"(oldCurrentCo->context.edx),
              "=m"(oldCurrentCo->context.esi), "=m"(oldCurrentCo->context.edi),
              "=m"(oldCurrentCo->context.ebp), "=m"(oldCurrentCo->context.esp)
            :
            :
            #endif
        );

        asm volatile(
            #if __x86_64__
            "mov $0, %%r9;"
            "leaq (%%rip), %%r8;"
            "cmp $0, %%r9;"
            "jne 0f;"
            "push %%r8;"

            "mov %%rsp, %0;"
            "mov %1, %%rsp;"
            "jmp *%2;"
            "0:\n\t"
            : 
            : "m"(oldCurrentCo->context.rsp),
              "r" (currentCo->stackBase),
              "r"(coroutine_wrapper)
            : "memory", "r9", "r8"
            #else
            "mov $0, %%esi;"
            "call 1f;"
            "1: cmp $0, %%esi;"
            "jne 0f;"
            "mov %%esp, %0;"

            "movl %1, %%esp;"
            "jmp *%2;"
            "0:\n\t"
            : "=m"(oldCurrentCo->context.esp)
            : "r"(currentCo->stackBase),
              "r"(coroutine_wrapper)
            : "memory"
            #endif
        );
    }
    return;
}


