int printf(const char*, ...);
int scanf(const char*, ...);
void free(void*);
void* malloc(long);

int main(void) {
    int *A;
    A = (int*) malloc(2 * sizeof(int));

    A[0] = A[1] = 8;

    printf("%d\n", 0[A]);

    A[1] = 2;

    printf("%d\n", A[2 != 4 - 1]);
    
    free(A);
    
    return 0;
}
