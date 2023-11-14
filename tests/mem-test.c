#include <stdlib.h>
#include <time.h>
#define RANDOM_NUM 25789432



void my_malloc() {
    srand(time(NULL));
    int* mem_addr = malloc(10 * sizeof(int));
    

    if (rand() == RANDOM_NUM) {
        mem_addr[0] = 1;
    } else {

    }


    void* b = calloc(1, sizeof(int));
    free(b);
    free(mem_addr);
}


int main() {

    int* a = (int*)malloc(sizeof(int));
    my_malloc();
    int b = 5;
    free(a);

    return EXIT_SUCCESS;
}
