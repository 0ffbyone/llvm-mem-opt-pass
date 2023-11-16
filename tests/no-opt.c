#include <stdlib.h>
extern void bar(void*);
extern void func();

int foo(int x) {
    void *a = malloc(1024);
    if (__builtin_expect(x == 42, 1)) {
        bar(a);
        return 1;
    } else {
        func();
    }

    return 0;
}

