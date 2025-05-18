#include "common.h"
#include "os.h"


// task number
uint16_t tid_cnt = 0;

task_t *task_lib[TASK_NUM_MAX];
task_t *task_current[CPU_NUM_MAX];

// use when w/r task_lib/tid_cnt
spinlock_t task_lk;
spinlock_t trap_lk;

typedef struct CPU
{
    int intena; //中断信息
    int noff;   //递归深度
} CPU;
CPU cpus[4];
static void push_off()
{
    int i = ienabled();
    iset(false);
    int c = cpu_current();
    if (cpus[c].noff == 0)
        cpus[c].intena = i;
    cpus[c].noff++;
}
static void pop_off()
{
    int c = cpu_current();
    assert(cpus[c].noff >= 1);
    cpus[c].noff--;
    if (cpus[c].noff == 0 && cpus[c].intena == true)
    {
        iset(true);
    }
}
static void spin_init(spinlock_t *lk, const char *name)
{
    lk->lock = 0;
    lk->cpu = -1;
    strcpy(lk->name, name);
}
static void spin_lock(spinlock_t *lk)
{
    while (atomic_xchg(&lk->lock, 1) != 0)
    {
        if(ienabled())
            yield();
    }
    for(volatile int i=0;i<10000;++i);
    push_off(); // disable interrupts to avoid deadlock.
    #ifdef delock
    //printf("thread %s : %s , cpu's intena:%d  \n",_current->name ,lk->name,cpus[cpu_current()].intena);
    //printf("thread %s : %s \n",_current->name ,lk->name);
    #endif
    lk->cpu = cpu_current();
}
static void spin_unlock(spinlock_t *lk)
{
    assert(lk->cpu == cpu_current());
    atomic_xchg(&lk->lock, 0);
    lk->cpu=-1;
    #ifdef delock
    //printf("%s  unlock\n", lk->name);
    #endif
    pop_off();
}


void kmt_sem_init(sem_t *sem, const char *name, int value)
{
    sem->name = name;
    sem->value = value;
    sem->queue_cnt = 0;
    kmt->spin_init(&(sem->lk),name);
    return;
}

void kmt_sem_wait(sem_t *sem)
{
    kmt->spin_lock(&sem->lk); // 获得自旋锁
    sem->value--; // 自旋锁保证原子性
    if (sem->value < 0) {
        // 没有资源，需要等待
        sem->queue[sem->queue_cnt] = task_current[cpu_current()];
        task_current[cpu_current()]->status = BLOCKED;
        // mark_as_not_runnable(current); // 当前线程不能再执行
    }

    kmt->spin_unlock(&sem->lk);
    if (sem->value < 0) {    // 如果 P 失败，不能继续执行
                        // (注意此时可能有线程执行 V 操作)
        yield();        // 引发一次上下文切换
    }
}

void kmt_sem_signal(sem_t *sem)
{
    kmt->spin_lock(&(sem->lk));
    sem->value++;
    if(sem->queue_cnt > 0) { // have waited queue
        assert(sem->queue[0] != NULL);
        sem->queue[0]->status = RUNNABLE;
        size_t i = 0;
        for (; i < sem->queue_cnt - 1; i++) {
            sem->queue[i] = sem->queue[i+1];
        }
        sem->queue[i] = NULL;
        sem->queue_cnt--;
    }
    kmt->spin_unlock(&(sem->lk));
}


static Context *kmt_context_save(Event ev, Context *ctx)
{
    task_current[cpu_current()]->context = *ctx;
    task_current[cpu_current()]->status = RUNNABLE;
    return NULL;
}


// no choose blocked
static Context *kmt_schedule(Event ev, Context *ctx)
{
    kmt->spin_lock(&task_lk);
    int task_cnt = 0;
    int able_cnt = 0;
    for (size_t i = 0; task_cnt < tid_cnt; i++) {
        if(task_lib[i] != NULL) {
            task_cnt ++;
            if(task_lib[i]->status == RUNNABLE)
                able_cnt++;
        }
    }
    // printf("task_cnt::%d\n",task_cnt);
    // printf("able_cnt::%d\n",able_cnt);
    int c = rand()%able_cnt + 1;
    for (size_t i = 0; i < TASK_NUM_MAX; i++) {
        if(task_lib[i] != NULL) {
            if(task_lib[i]->status == RUNNABLE) {
                c--;
                if(c == 0) {
                    task_lib[i]->status = RUNNING;
                    task_current[cpu_current()] = task_lib[i];
                    kmt->spin_unlock(&task_lk);
                    return &task_lib[i]->context;
                }
            }
        }
    }
    assert(0);
    return NULL;
}

task_t idle_task;

void idle_clean_func(void *arg)
{
    while (1) {
        kmt->spin_lock(&task_lk);
        for (size_t i = 0; i < tid_cnt; i++) {
            if(task_lib[i]->status == DEAD) {
                bool flag_use = false;
                for (size_t j = 0; j < CPU_NUM_MAX; j++)
                {
                    if(task_current[i] != NULL && task_current[j] == task_lib[i])
                        flag_use = true;
                }
                if(flag_use == false) {
                    pmm->free(task_lib[i]);
                    task_lib[i] = NULL;
                    tid_cnt--;
                }
            }
        }
        kmt->spin_unlock(&task_lk);
        yield();
    }
}

static void kmt_init(void)
{
    kmt->spin_init(&task_lk, "task_lk");
    kmt->spin_init(&trap_lk, "trap_lk");

    // idle task
    kmt->create(&idle_task, "idle", idle_clean_func, NULL);


    for (size_t i = 0; i < cpu_count(); i++)
    {
        if(i == 0 ) {
            task_current[i] = &idle_task;
            (&idle_task)->status = RUNNING;
            continue;
        }
        task_t *t = pmm->alloc(sizeof(task_t));
        kmt->create(t,"_",NULL,NULL);
        task_current[i] = t;
        t->status = RUNNING;
    }

    os->on_irq(INT_MIN, EVENT_NULL, kmt_context_save);
    os->on_irq(INT_MAX, EVENT_NULL, kmt_schedule);
    return;
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg)
{
    task->name = name;
    task->status = RUNNABLE;
    Area tstack = { .start = task->stack, .end = task + 1};
    kcontext(tstack, entry, arg);
    kmt->spin_lock(&task_lk);
    for (size_t i = 0; i < TASK_NUM_MAX; i++)
    {
        if(task_lib[i] == NULL)
        {
            task->tid = i;
            task_lib[i] = task;
            break;
        }
    }
    tid_cnt++;
    kmt->spin_unlock(&task_lk);
    return tid_cnt;
}

static void kmt_teardown(task_t *task)
{
    // task_lib[task->tid] = NULL;
    task->status = DEAD;
    for (size_t i = 0; i < CPU_NUM_MAX; i++)
    {
        // running in cpu
        if(task_current[i] != NULL && task->tid == task_current[i]->tid)
        {
            // running in teardown cpu
            if(i == cpu_current())
            {
                yield();
            }
            return;
        }
    }

    // dont running 
    kmt->spin_lock(&task_lk);
    task_lib[task->tid] = NULL;
    pmm->free(task);
    tid_cnt--;
    kmt->spin_unlock(&task_lk);
    return;
}


MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create  = kmt_create,
    .teardown  = kmt_teardown,
    .spin_init  = spin_init,
    .spin_lock  = spin_lock,
    .spin_unlock  = spin_unlock,
    .sem_init  = kmt_sem_init,
    .sem_wait  = kmt_sem_wait,
    .sem_signal  = kmt_sem_signal,
};
