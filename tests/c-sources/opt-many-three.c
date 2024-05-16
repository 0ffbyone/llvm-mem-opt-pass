#include <stdlib.h>

#define LIKELY(X) __builtin_expect(X, 1)
#define UNLIKELY(X) __builtin_expect(X, 0)

extern void bar(void*);
extern void func();
extern void foo_bar();

int foo(int x) {
    void *a = malloc(1024);
    void *b = malloc(2048);

    if (UNLIKELY(x == 42)) {
        bar(a);
    } else {
        func();
    }

    bar(b);
    return 0;
}

