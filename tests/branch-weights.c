#include <stdlib.h>
#include <time.h>

void foo() {
    srand(time(NULL));

    int* a = malloc(sizeof(int));
    int rand = 78432;
    if (__builtin_expect(rand == random(), 0)) {
        *a = 5;
    } else if (__builtin_expect(rand != random(), 1)) {
        int b = 5;
    }
    free(a);
}
