struct x {};
void f(void) {
    struct x *a;
    struct x **b;
    int **c;
    **b;
    &a;
    b = &a;
    (struct x*) c;
}
