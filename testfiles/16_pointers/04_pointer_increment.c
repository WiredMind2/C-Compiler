int main() {
    int a[2];
    a[0] = 4;
    a[1] = 5;
    int *p = a;
    p++;
    return *p; // 5
}
