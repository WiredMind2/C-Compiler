int main() {
	int a = 0;
	int b = 1;
	int t;
	for (int i = 0; i < 7; i++) {
		t = a + b;
		a = b;
		b = t;
	}
	return a;
}
