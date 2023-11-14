#include <stdlib.h>

void no_memalloc() {
}

void inside_malloc() {
    void* a = malloc(sizeof(int));
    free(a);
}

void* return_ptr() {
    return malloc(sizeof(int));
}
