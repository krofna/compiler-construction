void* malloc(long);
int printf(const char*, ...);

struct foo
{
    int x;
};

struct bar
{
    int y;
};

int main(void)
{
    int *p;
    p = (int*)malloc((long)(10 * sizeof(int)));
    int *q;
    q = 3 + p;
    q += 3;
    int mul;
    mul = 0 + 'A';
    *q = 420;
    int *r;
    r = q - 3;
    int x;
    x = p - q;
    printf("%d %d %d", p[6], mul, x);
}
