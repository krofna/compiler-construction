int printf(const char*, ...);

char f(void)
{
    printf("Hello World!\n");
}

int main(void)
{
    char (*ptr)(void);
    ptr = f;
    ptr();
}
