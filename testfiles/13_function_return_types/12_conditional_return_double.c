double absolute(double x) {
    if (x < 0.0) {
        return -x;
    } else {
        return x;
    }
}
int main() {
    return absolute(-3.14);
}

