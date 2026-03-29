// Variable used before scope exit
int main() {
    {
        int x = 42;
        x = x + 1;
    }
    return 42;
}

