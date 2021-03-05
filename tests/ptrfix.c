int printf(const char*, ...);

void f(int *x)
{
    printf("%d", x);
}

int main(void)
{
    int *x;
    int y;
    x = &y;
    f(x++);
    f(x++);
    f(++x);
    f(++x);
    f(x--);
    f(x--);
    f(--x);
    f(--x);
}
