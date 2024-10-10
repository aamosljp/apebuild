# ApeBuild

A single header C/C++ build system.

# Usage

Just include apebuild.h in one source file, see the top of that file for
more instructions.

You can also check the example in apebuild.c to see it in action. Build it using:
```bash
gcc -o apebuild apebuild.c
```
And then run apebuild:
```bash
./apebuild
```

# Contributing

See the below TODO list and see if you can implement any of those features, you
can also add features not in the list but those are not guaranteed to be merged
into this repository

# TODO

**NOTE: This is mainly for my personal projects, so I'm probably not going to
add features unless I need them myself**(see Contributing)

- [ ] Make cross-platform (Should work on windows now, haven't tested it though)
- [ ] Actually make building libraries work
