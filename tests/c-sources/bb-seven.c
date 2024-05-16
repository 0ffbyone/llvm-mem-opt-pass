#include <stdlib.h>

#define LIKELY(X) __builtin_expect(X, 1)
#define UNLIKELY(X) __builtin_expect(X, 0)

extern void bar(void*);
extern void func();
extern void foo_bar();

int foo(int x, int y) {
    void *a = malloc(1024);

    if (UNLIKELY(x == 42)) {
        if (y % 2 == 0) {
            foo_bar();
            bar(a);
            foo_bar();
        } else {
            func();
            bar(a);
            func();
        }
    } else {
        func();
    }

    return 0;
}
