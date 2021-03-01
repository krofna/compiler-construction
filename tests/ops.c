int g1, g2;
char* ptr;

struct s_t
{
    int x;
    int y;
    char *ptr;
} obj;

int puts(char*);

int main(void)
{
    int x, y, z, *p, *q;
    x = y + z;
    x = y * z;
    x = y / z;
    x = y % z;
    x = y << z;
    x = y >> z;
    x = y | z;
    x = y & z;
    x = y ^ z;
    x = ~y;
    x = -y;
    x += y;
    q = &y;
    *p = *q;
    g1 = 3;
    ptr = "keks";
    puts(ptr);
    struct s_t lobj;
}
