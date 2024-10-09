/* This file is an example on how to use apebuild.h */
#define APEBUILD_IMPL
#define APELD "gcc"
#define APELANGC
#define APECC "gcc"
#define APECFLAGS "-Wall", "-Werror", "-pedantic"
#define APECFLAGS_DEBUG "-ggdb"
#include "apebuild.h"

APEBUILD_MAIN(int argc, char **argv)
{
	APE_BUILD_EXEC("example");
	ape_build_sources_append_dir("example", "src");
	APE_BUILD_INCLUDES_APPEND("example", "include");
	ape_build_all_target(APE_TARGET_RELEASE);
	if (argc > 1 && strcmp(argv[1], "run") == 0)
		ape_run("example", argv + 2);
	return 0;
}
