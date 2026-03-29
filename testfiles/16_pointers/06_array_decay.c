int main() {
	int a[3];
	a[0] = 1;
	a[1] = 2;
	a[2] = 3;
	int s = 0;
	for (int i = 0; i < 3; i++) {
		s = s + a[i];
	}
	return s;
}
