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

#define INT_MIN 0
#define INT_MAX MAX_HANDLERS_PER_EVENT

#define SEM_QUEUE_MAX 5

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
    uint8_t     stack[4096];
};

struct spinlock
{
    int lock;
    int cpu;
    char name[20];
};

struct semaphore
{
    const char* name;
    int value;
    struct spinlock lk;
    struct task *queue[SEM_QUEUE_MAX];
    int queue_cnt;
};
