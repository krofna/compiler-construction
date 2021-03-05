int printf(const char*, ...);

void f()
{
    printf("f");
}

void g()
{
    printf("g");
}

int main(void)
{
    int x;
    x = (1 ? f() : g());
}
