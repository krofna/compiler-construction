int printf(const char*, ...);
void f(void)
{
    printf("hello world");
}
int main(void)
{
    int x;
    f(), x = 3;
}
