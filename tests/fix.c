int printf(const char*, ...);

void f(int x)
{
    printf("%d", x);
}

int main(void)
{
    int x;
    x = 3;
    f(x++);
    f(x++);
    f(++x);
    f(++x);
    f(x--);
    f(x--);
    f(--x);
    f(--x);

}
