// When we test your kernel implementation, the framework
// directory will be replaced. Any modifications you make
// to its files (e.g., kernel.h) will be lost. 

// Note that some code requires data structure definitions 
// (such as `sem_t`) to compile, and these definitions are
// not present in kernel.h. 

// Include these definitions in os.h.

#include <klib.h>
#include <klib-macros.h>

#define TASK_NUM_MAX 20
#define CPU_NUM_MAX 4

#define MAX_HANDLERS_PER_EVENT  256

#define INT_MIN -1
#define INT_MAX MAX_HANDLERS_PER_EVENT

#define SEM_QUEUE_MAX 10

typedef enum {
    RUNNING=0,
    RUNNABLE,
    BLOCKED,
    DEAD
}task_status;

struct task
{
    int         tid;
    const char  *name;
    task_status status;
    Context     context;
    uint8_t     stack[4096*2];
};

struct spinlock
{
    int status;
    int cpu;
    const char *name;
    // int irq_dis_depth;      // disable int cnt
    // bool irq_enble;          // origin int is enable?
};

// typedef struct semaphore
// {
//     int count;
//     char name[20];
//     struct spinlock lock;
//     // task_t* pool[64];//等待队列线程池
//     int l;//等待队列头
//     int r;//等待队列尾
// }semaphore;

struct semaphore {
    const char *name;
    int value;
    int queue_cnt;
    struct spinlock lk;
    struct task *queue[SEM_QUEUE_MAX];
};
