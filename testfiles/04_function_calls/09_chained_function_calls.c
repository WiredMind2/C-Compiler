int id(int x) { return x; }
int main() {
    return id(id(id(5)));
}
