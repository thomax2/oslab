#include <common.h>

void lock(lock_t *lk)
{
    while (atomic_xchg(&lk->status, 1));
    return;
}

void unlock(lock_t *lk)
{
    atomic_xchg(&lk->status,0);
    return;
}

void lock_init(lock_t *lk)
{
    lk->status=0;
}