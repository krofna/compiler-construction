void** (*f)(void**);

void g(void (*c)(void)) {

    c();
    return ;
}

void d(void) {

    return ;
}

int main(void) {
    g(d);

    return 0;
}
