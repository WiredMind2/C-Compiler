int main() {
    int a = 1;
    int *p1 = &a;
    int **p2 = &p1;
    **p2 = 42;
    return a;
}
