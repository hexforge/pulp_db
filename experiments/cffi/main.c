#include <stdio.h>

struct two_nums {
    int x;
    int y;
};

int add_struct(struct two_nums *thing);
int add_struct(struct two_nums *thing)
{
    printf("Result %d\n", thing->x+thing->y);
    return 0;
}


int add(int x, int y);
int add(int x, int y)
{
    printf("Result %d\n", x+y);
    return 0;
}


int main(int argc, char*argv[])
{
    printf("hello\n");
    add(1,2);
    return 0;
}
