#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;
static void *addr = NULL;

int rand(void)
{
    // RAND_MAX assumed to be 32767
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed)
{
    next = seed;
}

int abs(int x)
{
    return (x < 0 ? -x : x);
}

int atoi(const char *nptr)
{
    int x = 0;
    while (*nptr == ' ')
    {
        nptr++;
    }
    while (*nptr >= '0' && *nptr <= '9')
    {
        x = x * 10 + *nptr - '0';
        nptr++;
    }
    return x;
}


void *malloc(size_t size)
{
    // On native, malloc() will be called during initializaion of C runtime.
    // Therefore do not call panic() here, else it will yield a dead recursion:
    //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
    if (addr == NULL)
        addr = (void *)ROUNDUP(heap.start, 8);
    size = (size_t)ROUNDUP(size, 8);
    char *old = addr;
    addr = addr + size;
    assert((uintptr_t)heap.start <= (uintptr_t)addr && (uintptr_t)addr < (uintptr_t)heap.end);
    for (char *p = (char *)old; p != (char *)addr; p++)
        *p = 0;
    return old;

#endif
    return NULL;
}

void free(void *ptr)
{
}

#endif
