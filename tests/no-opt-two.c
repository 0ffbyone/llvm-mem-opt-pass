#include <stdlib.h>

#define LIKELY(X) __builtin_expect(X, 1)
#define UNLIKELY(X) __builtin_expect(X, 0)

extern void bar(void*);
extern void func();

int foo(int x) {
    void *a = malloc(1024);


    if (UNLIKELY(x == 50)) {
        bar(a);
    } else if (UNLIKELY(x == 42)) {
        bar(a);
    } else {
        func();
    }

    return 0;
}

