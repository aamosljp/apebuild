#ifndef _EXAMPLE_H_
#define _EXAMPLE_H_

#include <stddef.h>
#include <stdio.h>

static char *shift_args(int *argc, char ***argv) {
	if ((*argc) <= 0) return NULL;
	char *out = (*argv)[0];
	(*argv)++;
	(*argc)--;
	return out;
}

#endif
