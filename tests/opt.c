#include <stdlib.h>

#define LIKELY(X) __builtin_expect(X, 1)
#define UNLIKELY(X) __builtin_expect(X, 0)

extern void bar(void*);
extern void func();
extern void foo_bar();

int foo(int x) {
    void *a = malloc(1024);

    if (LIKELY(x == 0)) {
        func();
    }

    if (UNLIKELY(x == 100)) {
        foo_bar();
    }

    if (UNLIKELY(x == 42)) {
        bar(a);
        return 1;
    } else {
        func();
    }

    return 0;
}
