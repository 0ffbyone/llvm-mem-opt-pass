#include <stdlib.h>

#define LIKELY(X) __builtin_expect(X, 1)
#define UNLIKELY(X) __builtin_expect(X, 0)

extern void bar(void*);
extern void func();
extern void foo_bar();

int foo(int x, int y) {
    void *a = malloc(1024);

    if (UNLIKELY(x == 42)) { // 5
        // move here
        if (y % 2 == 0) { // 8
            if (UNLIKELY(x + y == 50)) { // 10
                bar(a);
            } else { // 11
                foo_bar();
            }
        } else { // 12
            bar(a);
        }
    } else { // 13
        func();
    }

    return 0;
}
