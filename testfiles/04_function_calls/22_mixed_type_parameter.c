int add(double pld_comp_is_cool, int times) {
    return pld_comp_is_cool + times;
}

double multiply(double pld_comp_is_cool, int times) {
    return pld_comp_is_cool * times;
}

int main() {
    double a = 23.2;
    return add(a, -3) + multiply(-a, 32);
}