int printf(char*, ...);
int scanf(char*, ...);

struct s_t
{
    int x;
    int y;
    int z;
};

struct s_t f(void)
{
    struct s_t obj;
    obj.x = 420;
    return obj;
}

int main(void)
{
    struct s_t obj, *ptr;
    ptr = &obj;
    scanf("%d", &obj.x);
    printf("%d %d", ptr->x, f().x);
}
