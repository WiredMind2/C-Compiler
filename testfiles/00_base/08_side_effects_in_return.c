int inc() { static int v = 0; v++; return v; }
int main() {
    return inc() + inc(); // should return 1+2=3
}
