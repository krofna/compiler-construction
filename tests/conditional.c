int main()
{
    int *p, *q;
    int x;
    p = 0;
    q = 1 ? p : 0;
    q = 1 ? 0 : p;
    x = 1 ? 1000 : 'a';
    x = 1 ? 'a' : 1000;
}
