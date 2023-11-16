#include <stdlib.h>

void foo(int x) {
    int* a = (int*)malloc(sizeof(int));

    if (__builtin_expect(x == 42, 0)) {
        *a = 5;
    }
}
