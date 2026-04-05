//@unsupported
int main() {
    int x = 42;
    int *p = &x;
    int **pp = &p;
    int ***ppp = &pp;
    return ***ppp;
}
