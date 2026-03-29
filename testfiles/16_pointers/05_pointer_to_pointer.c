int main() {
    int x = 8;
    int *p = &x;
    int **pp = &p;
    return **pp; // 8
}
