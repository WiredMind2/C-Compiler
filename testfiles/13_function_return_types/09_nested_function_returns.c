int nested1() {
    return 10;
}
int nested2() {
    return nested1() + 5;
}
int main() {
    return nested2();
}

