// #include <common.h>
#include "os.h"
#include <devices.h>
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
    pmm->init();
    kmt->init();    
    
    dev->init();

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
        
    }
    
    // while (1)
    // {
    //     // test0();
    //     test_repeated_alloc(64,64*3);
    //     test_repeated_alloc(512,8*3);
    //     test_repeated_alloc(4096,80);
    //     test_repeated_alloc(4096*4,80);
	// 	// test1();
	// 	// test2();
    // }
}

static Context *os_trap(Event ev, Context *context)
{
    Context *ret_ctx = NULL;    
    irq_handler_list *list = &irq_table;
    int cnt = list->cnt;

    // only one handler return a context
    for (int i = 0; i < cnt; i++) {
        irq_handler h = list->handlers[i];
        if(h.event == EVENT_NULL || h.event == ev.event) {
            Context *r = h.handler(ev, context);
            panic_on(r && ret_ctx, "return to multiple contexts");
            if (r) ret_ctx = r;
        }
    }
    panic_on(!ret_ctx, "return to NULL context");

    return ret_ctx;
}

static void os_on_irq(int seq, int event, handler_t handler)
{
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
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq
};
