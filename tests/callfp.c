int printf(const char*, ...);

void f(void)
{
    printf("hello world");
}

int main(void)
{
    void (*ptr)(void);
    ptr = f;
    ptr();
}
