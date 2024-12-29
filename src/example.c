#include "example.h"

int main(int argc, char **argv) {
	printf("Hello, World!\n");
	char *arg = shift_args(&argc, &argv);
	while(arg) {
		printf("%s\n", arg);
		arg = shift_args(&argc, &argv);
	}
	return 0;
}
