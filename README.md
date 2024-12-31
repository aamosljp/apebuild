# Apebuild: A Minimal Build System in C

Apebuild is a single-header, lightweight build system written in C.  
Currently, it's only used in a handful of my personal projects.
I mostly add features when I feel like it, but you are welcome to conribute.

# How It Works

You define build rules using the `APE_BUILDER` macro.  
Example:
```c
APE_BUILDER("example", {
    APE_INPUT_DIR("src/");
    APE_SET_FLAG(APE_FLAG_REBUILD);
});
```

Apebuild generates and runs commands dynamically based on your definitions.  

# Usage

```c
#define APEBUILD_IMPLEMENTATION
#define APE_PRESET_LINUX_GCC_C
#include "apebuild.h"

APEBUILD_MAIN(int argc, char **argv) {
    APE_BUILDER("example", {
        APE_INPUT_DIR("src/");
        APE_SET_FLAG(APE_FLAG_REBUILD);
    });
    return ape_run(argc, argv);
}
```

# TODO

- [ ] Add support for Windows toolchains.
- [ ] Improve library detection.
- [ ] Provide parallel build support.

# Contributions

Contributions are welcome!  
To contribute:
1. Fork the repository.
2. Create a feature branch.
3. Submit a pull request.

GitHub: [Apebuild Repository](https://github.com/aamosljp/apebuild)

# License

Apebuild is in the public domain.

