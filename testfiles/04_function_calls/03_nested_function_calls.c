int inner() {
    return 10;
}
int outer() {
    return inner() + 5;
}
int main() {
    return outer();
}
