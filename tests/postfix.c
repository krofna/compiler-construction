struct st
{
    int x;
};

void f(void)
{
}

int main()
{
    int x;
    x = 0;
    f();
    struct st obj;
    obj.x = 3;
    struct st *ptr;
    (ptr + 1)->x = 3;
    ptr->x++;
    ptr->x--;
    if (x + 1)
    {
    }
}
