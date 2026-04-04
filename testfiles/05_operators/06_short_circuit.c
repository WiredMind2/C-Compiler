//@unsupported
int side_effect(int *x) {
    *x = *x + 1;
    return 1;
}

int main() {
    int x = 0;
    int r1 = 0 && side_effect(&x);
    if (x != 0) return 1; // Side effect should NOT have happened

    int y = 0;
    int r2 = 1 || side_effect(&y);
    if (y != 0) return 2; // Side effect should NOT have happened

    return 0;
}
