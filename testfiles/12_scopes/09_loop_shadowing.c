// Loop with variable reuse and shadowing
int main() {
    int i = 0;
    int result = 0;
    while (i < 3) {
        int x = i * 10;
        {
            int x = i * 100;
            result = result + x;
        }
        result = result + x;
        i = i + 1;
    }
    return result;
}

