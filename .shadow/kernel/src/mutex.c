#include <common.h>

#define MAXLOCKNUM 100

void lock(lock_t *lk)
{
    size_t locknum = 0;
    while (atomic_xchg(&lk->status, 1))
    {
        locknum++;
        assert(0);
    }
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