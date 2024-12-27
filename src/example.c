 /*Source file for the apebuild.h example */
#include <example.h>
#include <stdio.h>

int main(int argc, char **argv) {
	printf("Hello World\n");
	while(argc > 0) {
		printf("%s\n", shift_args(&argc, &argv));
	}
	return 0;
}
