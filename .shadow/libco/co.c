/*
1. malloc & free complete but free too difficult
2. co_start
3. main initial co 
4. yield = save context + change context + 
*/
#include "co.h"
// #include <am.h>
// #include <klib.h>




coNode coMain = {
    .name = "main",
    .pid = 0,
    .next = NULL,
    .status = CO_RUNNING
};

coNode *currentCo = &coMain;

void insert_co(coNode *coNew)
{
    coNode *listEnd= &coMain;
    while (listEnd->next != NULL)
        listEnd = listEnd->next;
    listEnd->next = coNew;
    coNew->pid = listEnd->pid + 1;
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

void coroutine_wrapper(struct co *myCo) {
    printf("wrap\n");
    currentCo->status = CO_RUNNING;
    // printf("%s\n",myCo->name);
    // printf("%p\n",myCo->func);
    // printf("%p\n",myCo->arg);
    currentCo->func(currentCo->arg);
    currentCo->status = CO_DEAD;
    return;
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
    coNode *coNew = (coNode *)malloc(sizeof(coNode));
    // strcpy(coNew->name,name);
    coNew->name = name;
    coNew->func = func;
    // assert(coNew->func);
    coNew->arg = arg;
    coNew->next = NULL;
    coNew->status = CO_NEW;
    coNew->stackBase = (((uintptr_t)coNew->stack + DEFUALT_STACK_SIZE - 1) & (~(0xF)) )- 8;
    // coNew->stackBase = coNew->stack + DEFUALT_STACK_SIZE -8;
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
        num++;
        list = list->next;
    }
    return num;
}

coNode *oldCurrentCo;
coNode *newCurrentCo;

void co_yield() {
    int flag = 0;
    int coNum = get_coNum();

    int chooseNum = rand()%(coNum); // [0,coNum-1]
    printf("%d\t",chooseNum);
    newCurrentCo = &coMain ;
    for (size_t i = 0; i < chooseNum; i++)
        newCurrentCo = newCurrentCo->next;
    
    if(newCurrentCo == currentCo)
        return;
    oldCurrentCo = currentCo;
    currentCo = newCurrentCo;

    if (currentCo->status != CO_NEW)
    {
        asm volatile(
            #if __x86_64__
            "mov %%rax, %0;"
            "mov %%rbx, %1;"
            "mov %%rcx, %2;"
            "mov %%rdx, %3;"
            "mov %%rsi, %4;"
            "mov %%rdi, %5;"
            "mov %%rbp, %6;"
            "mov %%r8, %8;"
            "mov %%r9, %9;"
            "mov %%r10, %10;"
            "mov %%r11, %11;"
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
            "mov %%eax, %0;"
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

            "mov %1, %%rax;"
            "mov %2, %%rbx;"
            "mov %3, %%rcx;"
            "mov %4, %%rdx;"
            // "mov %5, %%rsi;"
            "mov %6, %%rdi;"
            "mov %7, %%rbp;"
            "mov %9, %%r8;"
            "mov $1, %%r9;"
            "mov %11, %%r10;"
            "mov %12, %%r11;"
            "mov %13, %%r12;"
            "mov %14, %%r13;"
            "mov %15, %%r14;"
            "mov %16, %%r15;"
            "mov %8, %%rsp;"
            "pop %%rcx;"
            "jmp *%%rcx;"
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
            "mov $0, %%eax;"
            "call 1f;"
            "1: cmp $0, %%eax;"
            "jne 0f;"
            "mov %%esp, %0;"

            "mov $1, %%eax;"
            "mov %2, %%ebx;"
            "mov %3, %%ecx;"
            "mov %4, %%edx;"
            "mov %5, %%esi;"
            "mov %6, %%edi;"
            "mov %7, %%ebp;"
            "mov %8, %%esp;"
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
            "mov %%rax, %0;"
            "mov %%rbx, %1;"
            "mov %%rcx, %2;"
            "mov %%rdx, %3;"
            "mov %%rsi, %4;"
            "mov %%rdi, %5;"
            "mov %%rbp, %6;"
            "mov %%r8, %8;"
            "mov %%r9, %9;"
            "mov %%r10, %10;"
            "mov %%r11, %11;"
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
            "mov %%eax, %0;"
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
            // "mov %3, %%rdi;"
            "jmp *%2;"
            "0:\n\t"
            : 
            : "m"(oldCurrentCo->context.rsp),
              "r" (currentCo->stackBase),
              "r"(coroutine_wrapper)
            //   "r"(currentCo)
            : "memory", "r9", "r8"
            #else
            "mov $0, %%eax;"
            "call 1f;"
            "1: cmp $0, %%eax;"
            "jne 0f;"
            "mov %%esp, %0;"

            "movl %1, %%esp;"
            "jmp *%2;"
            "0:\n\t"
            : "=m"(oldCurrentCo->context.esp)
            : "b"(currentCo->stackBase),
              "d"(coroutine_wrapper)
            : "memory"
            #endif
        );
    }
    return;
}


