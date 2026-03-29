int main() {
    int a = 1;
    a += (a += 2); // evaluate inner first: a becomes 3, then outer adds 3 -> 6
    return a;
}
