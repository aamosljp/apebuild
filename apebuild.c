#define APEBUILD_IMPLEMENTATION
#define APE_PRESET_LINUX_GCC_C
#define APE_LINK_ARGS(outfile) "-o", outfile
#include "apebuild.h"

int main(int argc, char **argv)
{
	APE_REBUILD(argc, argv);
	APE_BUILDER("example", {
		APE_INPUT_DIR_REC("src/");
		if (argc > 1 && (strcmp(argv[1], "--rebuild") == 0 ||
				 strcmp(argv[1], "-r") == 0)) {
			APE_SET_FLAG(APE_FLAG_REBUILD);
			argv++;
			argc--;
		}
		if (argc > 1 && strcmp(argv[1], "run") == 0) {
			APE_SET_FLAG(APE_FLAG_RUN_AFTER_BUILD);
		}
	});
}
