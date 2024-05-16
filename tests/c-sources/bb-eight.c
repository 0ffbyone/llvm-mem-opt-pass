#include <stdlib.h>

#define LIKELY(X) __builtin_expect(X, 1)
#define UNLIKELY(X) __builtin_expect(X, 0)

extern void bar(void*);
extern void func();
extern void foo_bar();

int foo(int x, int y) {
    void *a = malloc(1024);

    if (x % 2) {
        if (UNLIKELY(y == 42)) {
            bar(a);
        } else {
            func();
        }
        } else if (UNLIKELY(y == 420)){
            bar(a);
        } else {
            foo_bar();
    }

    return 0;
}
