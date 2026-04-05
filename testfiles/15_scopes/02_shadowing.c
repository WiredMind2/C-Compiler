int main() {
    int x = 1;
    {
        int x = 2;
        if (x != 2) return 1;
    }
    if (x != 1) return 2;
    return 0;
}
