int g(int);

int f(int x) {
    if(x == 0) return 1;
    return x * f(x - 1);
}

int e(int x) {
    return g(x);
}

int g(int x) {
    int i, mx;
    mx = 1;
    for(i = 2; i <= x; ++i)
        mx *= i;
    
    return mx;
}

int main(void)
{
    printf("%d %d", f(10), g(10));
}
