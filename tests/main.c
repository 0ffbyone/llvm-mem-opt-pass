#include <stdio.h>

void bar(void* ptr) {
    printf("called void bar(%p)\n", ptr);
}

void func() {
    printf("called void func()\n");
}

void foo_bar() {
    printf("called void foo_bar()\n");
}

void f() {
    printf("called void f()\n");
}

extern int foo(int x);



int main() {
    int number;
    scanf("%d", &number);
    foo(number);
}
