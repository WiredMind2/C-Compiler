int isprime(int n) { if (n<2) return 0; for (int i=2;i*i<=n;i++) if (n%i==0) return 0; return 1; }
int main() { int c=0; for (int i=2;i<=10;i++) c += isprime(i); return c; }
