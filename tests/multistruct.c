int printf(char*, ...);

struct type
{
    int x;
    int y;
};

int main(void)
{
    struct type obj, *ptr;
    obj.x = 3;
    ptr = &obj;
    ptr->y = 4;
    printf("%d", obj.x * obj.y);
}
