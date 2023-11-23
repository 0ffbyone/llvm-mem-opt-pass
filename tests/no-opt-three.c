#include <stdlib.h>

#define LIKELY(X) __builtin_expect(X, 1)
#define UNLIKELY(X) __builtin_expect(X, 0)

extern void bar(void*);
extern void func();
extern void foo_bar();
extern void f();


int foo(int x, int y) {
    void *a = malloc(1024);


    if (UNLIKELY(x == 50)) {
        bar(a);
    } else if (UNLIKELY(x == 42)) {
        foo_bar();
    }

    if (UNLIKELY(y == 50)) {
        bar(a);
    } else if (UNLIKELY(y == 42)) {
        func();
    }

    return 0;
}

