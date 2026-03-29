int multiply_recursive(int x, int count) {
    if (count == 0) {
        return 1;
    } else {
        return x * multiply_recursive(x, count - 1);
    }
}
int main() {
    return multiply_recursive(2, 3);
}

