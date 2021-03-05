int printf(const char*, ...);
int scanf(const char*, ...);
void free(void*);
void* malloc(long);

struct node
{
    int x;
    struct node* next;
};

struct node* new_node(int x)
{
    struct node* ptr;
    ptr = malloc(sizeof(struct node));
    ptr->x = x;
    return ptr;
}

struct node* remove(struct node* f, int x)
{
    struct node** s, **r;
    s = &f;
    r = &f;
    while (*r)
    {
        if ((*r)->x == x)
        {
            struct node* temp;
            temp = (*r)->next;
            free(*r);
            *r = temp;
        }
        else
            r = &(*r)->next;
    }
    return *s;
}

void print_list(struct node* r)
{
    struct node *i;
    for (i = r; i; i = i->next)
        printf("%d ", i->x);
    printf("\n");
}

struct node* make_list(int n)
{
    struct node *r, *l;
    r = l = 0;
    int i;
    for (i = 0; i < n; ++i)
    {
        int x;
        scanf("%d", &x);
        if (!r)
        {
            r = new_node(x);
            l = r;
        }
        else
        {
            l->next = new_node(x);
            l = l->next;
        }
    }
    return r;
}

struct node* make_list_better(int n)
{
    struct node *first, **r;
    first = 0;
    r = &first;
    int i;
    for (i = 0; i < n; ++i)
    {
        int x;
        scanf("%d", &x);
        *r = new_node(x);
        r = &(*r)->next;
    }
    return first;
}

void delete_list(struct node* r)
{
    struct node *i;
    for (i = r; i;)
    {
        struct node* j;
        j = i->next;
        free(i);
        i = j;
    }
}

int main()
{
    int n;
    scanf("%d", &n);
    struct node* r;
    r = make_list_better(n);
    int m;
    scanf("%d", &m);
    int i;
    for (i = 0; i < m; ++i)
    {
        int x;
        scanf("%d", &x);
        r = remove(r, x);
        print_list(r);
    }
    delete_list(r);
}
