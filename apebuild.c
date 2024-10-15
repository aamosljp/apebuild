/* This file is an example on how to use apebuild.h */

/* Define APEBUILD_IMPL, this is required for the rest to work */
#define APEBUILD_IMPL
/* Define gcc as the linker */
#define APELD "gcc"
/* Compile C source files */
#define APELANGC
/* Define gcc as the compiler */
#define APECC "gcc"
/* Add some custom flags for the compiler */
#define APECFLAGS "-Wall", "-Werror"
/* Add more flags but only with APE_TARGET_DEBUG */
#define APECFLAGS_DEBUG "-ggdb"
#include "apebuild.h"

/* The custom main "function" */
APEBUILD_MAIN(int argc, char **argv)
{
	/* Tell apebuild to build the executable "example" */
	APEINFO("Adding \"example\" to builds");
	APE_BUILD_EXEC("example");
	/* Add every file in the "src" directory to "example" recursively */
	APEINFO("Appending directory \"src\" to \"example\" sources");
	ape_build_sources_append_dir("example", "src");
	/* Add "include" as an include directory */
	APEINFO("Appending directory \"include\" to \"example\" as an include "
		 "directory");
	APE_BUILD_INCLUDES_APPEND("example", "include");
	/* Finally, build everything with APE_TARGET_RELEASE */
	APEINFO("Building all with target APE_TARGET_RELEASE");
	ape_build_all_target(APE_TARGET_RELEASE);
	/* And optionally run "example" immediately after building */
	if (argc > 1 && strcmp(argv[1], "run") == 0) {
		APEINFO("Running \"example\"...");
		ape_run("example", argv + 2);
	}
	return 0;
}
