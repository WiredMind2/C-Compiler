//@unsupported
union data {
    int i;
    char c;
};

int main() {
    union data d;
    d.i = 65;
    return d.c;
}
