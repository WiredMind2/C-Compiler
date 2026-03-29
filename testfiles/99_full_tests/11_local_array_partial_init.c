// Test local array with partial initializer - remaining elements should be zero
int main() {
    int a[3] = {1};
    return a[0] + a[1] + a[2];
}
