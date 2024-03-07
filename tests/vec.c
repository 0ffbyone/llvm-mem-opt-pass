#include <stdlib.h>

#define LIKELY(X) __builtin_expect(X, 1)
#define UNLIKELY(X) __builtin_expect(X, 0)

extern void bar(void*);
extern void func();
extern void foo_bar();

int foo(int x) {
    int *a = (int*)malloc(1024 * sizeof(int));

    if (UNLIKELY(x == 42)) {
        bar(&a[10]);
        bar(&a[20]);
        bar(&a[30]);
        bar(&a[40]);
        bar(&a[50]);
        return 1;
    } else {
        func();
    }

    return 0;
}
