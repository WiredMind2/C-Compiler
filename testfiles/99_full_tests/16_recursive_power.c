// Recursive integer power: returns 2^6 + 3^3 = 64 + 27 = 91
int power(int base, int exp) {
    if (exp == 0) return 1;
    return base * power(base, exp - 1);
}

int main() {
    return power(2, 6) + power(3, 3);
}
