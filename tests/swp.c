int printf(const char *c, ...);

void swp(int *x, int* y) {
    int c = *x;
    *x = *y;
    *y = c;

    return ;
}

int main(void) {
    int x, y;
    x = 3;
    y = 15;
    x = x += 3;
    printf("%d %d\n", x, y);
    swp(&x, &y);
    printf("%d %d\n", x, y);

    return 0;
}
