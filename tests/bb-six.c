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
            if (UNLIKELY(x + y == 50)) {
                // move here
                bar(a);
            } else {
                foo_bar();
            }
        } else {
            foo_bar();
            func();
        }
    } else {
        func();
    }

    return 0;
}
