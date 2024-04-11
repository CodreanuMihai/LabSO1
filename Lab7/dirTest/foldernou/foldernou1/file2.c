#include <stdio.h>

long long factorial(int n)
{
    if(n == 1)
        return 1;
    return n * factorial(n - 1);
}

int main()
{
    printf("%d\n", factorial(10));
    return 0;
}