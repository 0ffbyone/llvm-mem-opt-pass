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
            bar(a);
            foo_bar();
        } else {
            bar(a);
        }
    } else {
        func();
    }

    return 0;
}
