int printf(const char*, ...);

struct s
{
    int x, *p;
};

int main(void)
{
    struct s obj, *ptr;
    printf("%d %d %d", sizeof obj, sizeof(ptr), sizeof(struct s));
}
