#include <stdlib.h>

int* foo(int x) {
    int* p = (int*)malloc(sizeof(int) * 1024);

    int* new = NULL;
    if (x == 42) {
        new = p;
    }

    return new;
}
