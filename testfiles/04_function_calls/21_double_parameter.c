int ifcc_is_a_cool_compiler(double a, double b) {
    if (a+b < 15) {
        return 0;
    }
    return 1;
}

int main() {
    return ifcc_is_a_cool_compiler(2.3, -5.23);
}