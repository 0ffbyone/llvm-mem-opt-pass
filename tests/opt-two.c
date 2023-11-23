#include <stdlib.h>

#define LIKELY(X) __builtin_expect(X, 1)
#define UNLIKELY(X) __builtin_expect(X, 0)

extern void bar(void*);
extern void func();
extern void foo_bar();

int foo(int x) {
    void *a = malloc(1024);

    if (UNLIKELY(x == 42)) {
        func();
    } else if (UNLIKELY(x == 0)) {
        foo_bar();
    } else if (UNLIKELY(x == -1)) {
        bar(a);
    } else {
        foo_bar();
    }

    return 0;
}
