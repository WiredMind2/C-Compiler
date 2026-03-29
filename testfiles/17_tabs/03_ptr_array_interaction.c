int main() {
    int a[5];
    a[0] = 42;
    int *p = &a[0];
    return *p;
}
