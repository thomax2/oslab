// #include <common.h>
#include "os.h"
#include <devices.h>
// #include <kernel.h>

// seq in all event
typedef struct 
{
    int seq;
    int event;
    handler_t handler;
}irq_handler;

typedef struct 
{
    int cnt;
    irq_handler handlers[MAX_HANDLERS_PER_EVENT];
}irq_handler_list;

irq_handler_list irq_table;

extern task_t *task_current[CPU_NUM_MAX];
extern spinlock_t trap_lk;

static void tty_reader(void *arg) {
    device_t *tty = dev->lookup(arg);
    char cmd[128], resp[128], ps[16];
    snprintf(ps, 16, "(%s) $ ", arg);
    while (1) {
        tty->ops->write(tty, 0, ps, strlen(ps));
        int nread = tty->ops->read(tty, 0, cmd, sizeof(cmd) - 1);
        cmd[nread] = '\0';
        sprintf(resp, "tty reader task: got %d character(s).\n", strlen(cmd));
        tty->ops->write(tty, 0, resp, strlen(resp));
    }
}

static inline task_t *task_alloc() {
    return pmm->alloc(sizeof(task_t));
}

static void os_init() {
    irq_table.cnt = 0;
    memset(irq_table.handlers, 0, sizeof(irq_table.handlers));
    pmm->init();
    kmt->init();    
    // printf("kmt init success\n");
    dev->init();
    // printf("dev init success\n");
    kmt->create(task_alloc(), "tty_reader", tty_reader, "tty1");
    kmt->create(task_alloc(), "tty_reader", tty_reader, "tty2");
}

static void os_run() {
    for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
        putch(*s == '*' ? '0' + cpu_current() : *s);
    }

    iset(true);
    while (1)
    {
        // yield();
        // printf("?\n");
    }
    
    // while (1)
    // {
    //     // test0();
    //     // test_repeated_alloc(64,64*3);
    //     // test_repeated_alloc(512,8*3);
    //     // test_repeated_alloc(4096,80);
    //     test_repeated_alloc(8*1024*1024,1);
    //     test_repeated_alloc(4*1024*1024,1);
    //     test_repeated_alloc(2*1024*1024,4);
	// 	// test1();
	// 	// test2();
    // }
}

static Context *os_trap(Event ev, Context *context)
{
    kmt->spin_lock(&trap_lk);
    Context *ret_ctx = NULL;    
    irq_handler_list *list = &irq_table;
    int cnt = list->cnt;
    // printf("GP Fault: cause=0x%x, ref=0x%x, msg=%s\n", ev.cause, ev.ref, ev.msg);
    // for (size_t i = 0; i < cnt; i++)
    // {
    //     printf("list->handlers[i].seq::%d\n",list->handlers[i].seq);
    // }
    
    // only one handler return a context
    for (int i = 0; i < cnt; i++) {
        irq_handler h = list->handlers[i];
        // assert(h.handler != NULL);
        // if(h.handler == NULL)
        //     printf("waaaaaaa %d %d\n",cnt,h.seq);
        // printf("jjjjjjjj%d\n",h.seq);
        if(h.event == EVENT_NULL || h.event == ev.event) {
            Context *r = h.handler(ev, context);
            panic_on(r && ret_ctx, "return to multiple contexts");
            // if (r && ret_ctx) {
            //     // printf("os_trap: multiple handlers returned context!\n");
            //     // printf("  Event: (event=%d)\n", ev.event);
            //     // printf("  cause=0x%p, ref=0x%p, msg=%s\n", ev.cause, ev.ref, ev.msg ? ev.msg : "(null)");
            //     panic("return to multiple contexts");
            // }
            if (r) ret_ctx = r;
        }
    }
    panic_on(!ret_ctx, "return to NULL context");
    kmt->spin_unlock(&trap_lk);
    return ret_ctx;
}

static void os_on_irq(int seq, int event, handler_t handler)
{
    assert(handler != NULL);
    irq_handler_list *list = &irq_table;
    
    assert(list->cnt <  MAX_HANDLERS_PER_EVENT);

    // insert order
    int i = list->cnt - 1;
    while (i >= 0 && list->handlers[i].seq > seq)
    {
        list->handlers[i + 1] = list->handlers[i];
        i--;
    }
    list->handlers[i+1].seq = seq;
    list->handlers[i+1].handler = handler;
    list->handlers[i+1].event = event;
    list->cnt ++;
    return;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq
};
