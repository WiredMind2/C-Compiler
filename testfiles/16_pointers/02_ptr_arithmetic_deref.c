int main() {
    int a = 10;
    int b = 20;
    int *p1 = &a;
    int *p2 = &b;
    int res = (*p1) + (*p2);
    return res;
}
