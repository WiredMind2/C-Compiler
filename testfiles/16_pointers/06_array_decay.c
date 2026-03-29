int sum(int *a, int n) { int s=0; for (int i=0;i<n;i++) s+=a[i]; return s; }
int main() { int a[3]={1,2,3}; return sum(a,3); }
