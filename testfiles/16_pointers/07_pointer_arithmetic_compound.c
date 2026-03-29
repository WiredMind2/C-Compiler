int main() {
    int a[4];
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    a[3] = 4;
    int *p = a + 1;
    p += 2; // points to a[3]
    return *p; // 4
}
