int printf(const char*, ...);
void* malloc(int);

int main(void)
{
    int *p;
    p = malloc(10 * sizeof(int));
    int *q;
    q = p + 10;
    int x;
    x = 'a';
    printf("%d\n", p - q);
    printf("%d %d %d %d", sizeof(char), sizeof(int), sizeof(char*), sizeof(int*));
}
