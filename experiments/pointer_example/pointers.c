#include <stdio.h>

char z=12;

void foo(char **x);
void foo(char **x)
{
    *x=&z;
}


void foo2(char *x);
void foo2(char *x)
{
    printf("foo2 x pre %p\n", x);
    x=x+1;
    printf("foo2 x post %p\n", x);
}

int main()
{
    char *x;
    printf("pointer x value uninited %p\n", x);
    printf("-------\n");
    x = NULL;
    printf("pointer x value nulled %p\n", x);
    printf("-------\n");
    char y = 10;
    x = &y;
    printf("address of y %p\n", &y);
    printf("address pointed by x %p\n", x);
    printf("value of x dereferenced %d\n", *x);
    printf("-------\n");
    x = NULL;
    foo(&x);
    printf("address pointed by z %p\n", &z);
    printf("address pointed by x %p\n", x);
    printf("val pointed by x %d\n", *x);
    printf("-------\n");
    foo2(x);
    printf("address pointed by x %p\n", x);


}
