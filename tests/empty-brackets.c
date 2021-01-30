void f(void) {

    int x;
    (); // needs to fail
    {();} // <- segfault in --print-ast, needs to fail

    return ;
}
