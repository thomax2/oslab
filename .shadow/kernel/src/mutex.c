#include <common.h>

size_t locknum = 0;
#define MAXLOCKNUM 10000000

void lock(lock_t *lk)
{
    while (atomic_xchg(&lk->status, 1))
    {
        locknum++;
        assert(locknum < MAXLOCKNUM);
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