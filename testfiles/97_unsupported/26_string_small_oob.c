//@unsupported
// Reason: Out-of-bounds access on string literals.
int main() {
    int i = "abc"[100];
    return i;
}
