#define APEBUILD_IMPLEMENTATION
#define APE_PRESET_LINUX_GCC_C
#define APE_LINK_ARGS(outfile) "-o", outfile
#include "apebuild.h"

APEBUILD_MAIN(int argc, char **argv)
{
	APE_BUILDER("example", {
		APE_INPUT_DIR_REC("src/");
		while (argc > 0) {
			char *arg = argv[0];
			if (strcmp(arg, "--rebuild") == 0)
				APE_SET_FLAG(APE_FLAG_REBUILD);
			if (strcmp(arg, "--watch") == 0)
				APE_WATCH();
			argv++;
			argc--;
		}
		/*if (argc > 1 && (strcmp(argv[1], "--rebuild") == 0 ||*/
		/*		 strcmp(argv[1], "-r") == 0)) {*/
		/*	APE_SET_FLAG(APE_FLAG_REBUILD);*/
		/*	argv++;*/
		/*	argc--;*/
		/*}*/
		/*if (argc > 1 && strcmp(argv[1], "run") == 0) {*/
		/*	APE_SET_FLAG(APE_FLAG_RUN_AFTER_BUILD);*/
		/*}*/
	});
	return 0;
}
