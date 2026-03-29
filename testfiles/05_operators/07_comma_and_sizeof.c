int f() { return 9; }
int main() { int a; (a=1, a+=2); return sizeof(a) + f(); }
