#include <stdlib.h>

#define LIKELY(X) __builtin_expect(X, 1)
#define UNLIKELY(X) __builtin_expect(X, 0)

extern void bar(void*);
extern void func();

int foo(int x) {
    void *a = malloc(1024);

    if (UNLIKELY(x == 42)) {
        bar(a);
    }

    func();
    bar(a);

    return 0;
}

