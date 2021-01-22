void *f(void) {
    int *a;
    
    (int) a;
    (int*)(int) a;
    *(int**)a;
    return (void*) a;
}
