struct lel ((((*g(void)))))
{
}

struct lel
{
    int x;
} f(void)
{
    struct lel lol;
    lol.x = 3;
    return lol;
}

int main()
{
    f();
    void* b;
    ((struct lel {int a; int b;}*) b)->a;
    struct lel lol;
    lol.a;
}
