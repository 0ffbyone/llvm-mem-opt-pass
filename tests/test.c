#include <stdlib.h>

extern int* bar(int* ptr);

int* foo(int x, int y) {
    int* p = (int*)malloc(sizeof(int) * 1024);

    int* new = NULL;
    if (x == 42) {
        new = bar(p);
    }

    return new;
}
