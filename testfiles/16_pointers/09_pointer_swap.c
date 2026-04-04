// Classic swap via pointers
void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int main() {
    int x = 10;
    int y = 42;
    swap(&x, &y);
    return x; // should be 42
}
