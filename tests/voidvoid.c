int printf(const char*, ...);

void f(void) {
    printf("8\n");
    
    return ;
}

void g(void) {
    printf("g(8)\n");

    return f(); // ISO forbids, --pedantic gives warning
}

int main(void) {
    g();
    
    return 0;
}
