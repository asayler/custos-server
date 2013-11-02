#include <stdlib.h>
#include <stdio.h>

void myfree(int** in) {

    int* a;

    printf("1: in  = %p\n", in);
    printf("2: *in = %p\n", *in);

    a = *in;

    printf("3: a   = %p\n", a);

    free(a);

    printf("4: in  = %p\n", in);
    printf("5: *in = %p\n", *in);
    printf("6: a   = %p\n", a);

    *in = NULL;

    printf("7: in  = %p\n", in);
    printf("8: *in = %p\n", *in);
    printf("9: a   = %p\n", a);

}

int main() {

    int* a;

    printf("Test\n");

    a = malloc(sizeof(*a));

    printf("Before: %p\n", a);
    myfree(&a);
    printf("After: %p\n", a);

    return 0;

}
