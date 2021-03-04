int* malloc(int);
int printf(char*, ...);

int main(void)
{
    int *x;
    x = malloc(4 * 20);
    int i;
    for (i = 0; i < 20; ++i)
        x[i] = i + x[i - 1];;

    for (i = 0; i < 20; ++i)
        printf("%d ", x[i]);
}
