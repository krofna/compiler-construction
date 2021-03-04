int printf(const char*, ...);

int main()
{
    int x;
    x = 3;
    if (x == 3)
    {
        goto brk;
    }
kek:
    x = 4;
    printf("%d", x);
    return 0;
brk:
    printf("%d", x);
    goto kek;
}
