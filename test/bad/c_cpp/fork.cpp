#include <unistd.h>

int main() {
	for (int i = 0; i < 100; i++) fork();
	return 0;
}