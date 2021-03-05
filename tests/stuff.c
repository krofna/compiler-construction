int printf(const char*, ...);

int main(void)
{
    if (0 || 1)
    {
        printf("rip");
    }
    if (1 || 0)
    {
        printf("rip");
    }
    if (1 && 0)
    {
        printf("rip");
    }
    if (0 && 1)
    {
        printf("rip");
    }
    if (1 && 1)
    {
        printf("rip");
    }
    if (0 && 0)
    {
        printf("rip");
    }

}
