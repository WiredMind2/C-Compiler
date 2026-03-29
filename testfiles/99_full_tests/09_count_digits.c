int main() {
	int n = 12345;
	int c = 0;
	while (n) {
		n = n / 10;
		c = c + 1;
	}
	return c;
}
