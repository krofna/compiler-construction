int printf(const char*, ...);

void f(void)
{
    printf("hello ");
}

void* g(void)
{
    printf("world");
    return 0;
}

int main(void)
{
    f();
    g();
    void *ptr;
}    
