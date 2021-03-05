int printf(const char*, ...);

void f(void(*g)(void))
{
    g();
}

void blah(void)
{
    printf("hello world");
}

void (*h(void))(void)
{
    return blah;
}

int main(void)
{
    f(h());
}
