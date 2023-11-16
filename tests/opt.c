#include <stdlib.h>
extern void bar(void*);

int foo(int x) {
    void *a = malloc(1024);
    if (__builtin_expect(x == 42, 0)) {
        bar(a);
        return 1;
    }

    return 0;
}


