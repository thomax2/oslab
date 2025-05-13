#include <common.h>


static void os_init() {
    pmm->init();
}

static void os_run() {
    for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
        putch(*s == '*' ? '0' + cpu_current() : *s);
    }
    while (1)
    {
        // test0();
        test_repeated_alloc(64,64*3);
        test_repeated_alloc(512,16*3);
        test_repeated_alloc(4096,256);
        test_repeated_alloc(4096*4,8*64);
		// test1();
		// test2();
    }
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
};
