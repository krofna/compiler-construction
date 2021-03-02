struct bla
{
    int x;
    int y;
    int z;
};

struct bla f(int x)
{
    struct bla kek;
    kek.x = x;
    return kek;
}

void g(int x)
{
}

int main(void)
{
    g((f)(3).x);
    g((f)(3).x);
}
