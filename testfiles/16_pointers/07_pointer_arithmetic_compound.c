int main() {
    int a[4] = {1,2,3,4};
    int *p = a + 1;
    p += 2; // points to a[3]
    return *p; // 4
}
