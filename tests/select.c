void f(void) {
    int x;
    x = 4;
    switch(2) {
    case 1:
        break;
    case 2:
        ;
    case (3):;
    case (3 + 2):
        ;
    case x: // <- unexpected pass
        ;
    default:
        ;
    default: // <- two defaults is wrong, unexpected pass
        ;
    }
    
    return ;
}
