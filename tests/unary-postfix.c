void f(void) {
    int x;
    // fixed segfault
    //()++; // <- segfault
    // everything else seems to be ok
    x++;
    x--;
    x+++++0;
    *x++;
    &x++;
    **x++;
    x+++x--;
    (x)++;
    *(x++);
    ++0++;
    --++0++--;
    x +++= 1;
    x +=x++;
    x += x ++ += x ++;
    x << x ++;
    x += ++ x += x;
    f(x)++;
    f(x++ += x);
    x |= x += x &= x ++ ^= x %= x++;
    sizeof *&!-x;
    x = - x;
    x << x ++;
    x ++ << x ++;
    x ++ + x ++;
    sizeof x ++;
    - x ++;
    - x --;
    + x ++;
    f(x++, x++);
    f(f(x++)++, f(x)++++++++);
    (x += x)++;
    return ;
}
