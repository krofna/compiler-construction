int scanf(char*, ...);
int printf(char*, ...);

int fib(int n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

int main(void)
{
    int x;
    scanf("%d", &x);
    int i;
    for (i = 1; i <= x; ++i)
        printf("%d ", fib(i));
}
