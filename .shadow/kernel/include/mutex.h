#ifndef MUTEX_H__
#define MUTEX_H__

typedef struct mutex
{
    int status;
} lock_t;

void lock(lock_t *lk);
void unlock(lock_t *lk);
void lock_init(lock_t *lk);

#endif