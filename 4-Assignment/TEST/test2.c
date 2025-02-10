#include <stdio.h>
void foo(int a, int b)
{
    int c = a, d = b;
    for (int i = 1; i < 10; i++)
        c += i;

    for (int j = 0; j < 10; j++)
        d += j;

    printf("%d %d\n", c, d);
}

int main()
{
    foo(10, 4);
    foo(7, 12);
    return 0;
}
