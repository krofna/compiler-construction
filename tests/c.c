
int main()
{
    void* b;
    void (*g)(int, int);
    g = (void (*)(int, int)) b;
    sizeof(void(*)(void));
}
