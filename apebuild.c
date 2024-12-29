#define APEBUILD_IMPLEMENTATION
/*#define APE_PRESET_LINUX_GCC_C*/
#define APECC "gcc"
#define APELD "gcc"
#define APE_SRC_EXTENSION ".c"
#define APE_OBJ_EXTENSION ".o"
#define APE_BUILD_SRC_ARGS(infile, outfile) "-c", infile, "-o", outfile
#define APE_LINK_ARGS(outfile) "-o", outfile
#define APE_LINK_ARGS_ADD_LIB(lib) "-l" lib
#define APE_LINK_ARGS_ADD_LIBDIR(dir) "-L" dir
#define APE_LINK_ARGS_SHARED_LIB(outfile) "-shared", "-fPIC", "-o", outfile
#define APE_LIB_PREFIX "lib"
#define APE_LIB_SUFFIX ".so"
#include "apebuild.h"

// Global configuration options
#define APE_OUTPUT_DIR "build/"

// The apebuild main function macro, you should always use this instead of int main()
APEBUILD_MAIN(int argc, char **argv)
{
	// Builder for a standalone application
	APE_BUILDER("example", {
		APE_INPUT_DIR("src/"); // Add source files from "src/"
		APE_INCLUDE_DIR("include/"); // Add include directory
	});

	// Run the builder
	return ape_run(argc, argv);
}
