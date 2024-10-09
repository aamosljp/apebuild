/* Include file for the apebuild.h example */
#ifndef _EXAMPLE_H
#define _EXAMPLE_H

#include <assert.h>

static char *shift_args(int *argc, char ***argv) {
	assert((*argc) > 0);
	char *arg = (*argv)[0];
	(*argc)--;
	(*argv)++;
	return arg;
}

#endif
