//@unsupported
struct point {
    int x;
    int y;
};

int main() {
    struct point p;
    p.x = 1;
    p.y = 2;
    return p.x;
}
