/*
	apebuild.h -- Build system entirely in one header (currently linux only)

	Project URL: https://github.com/aamosljp/apebuild

	If you would like to contribute, see README.md

	Do this:
		#define APEBUILD_IMPL
	before you include this file in *one* C or C++ file to create the
	implementation.

	In the same place, also define your linker command:
		#define APELD "gcc"

	Also define one or both of:
		#define APELANGC		-- Compile C files
		#define APELANGCXX		-- Compile C++ files

	If APELANGC is defined, you can optonally define:
		#define APECFLAGS		-- Custom C compiler flags, quoted strings separated by commas
		#define APECFLAGS_DEBUG		-- Same as above, but only applies with APE_TARGET_DEBUG

	If APELANGCXX is defined, you can optonally define:
		#define APECXXFLAGS		-- Custom C++ compiler flags, quoted strings separated by commas
		#define APECXXFLAGS_DEBUG	-- Same as above, but only applies with APE_TARGET_DEBUG


	Custom data structures:

		NOTE: not all custom types are listed here, if you want, you can always just
		read through the code yourself

		Dynamic arrays:
			There are several types of dynamic arrays defined within this header,
			you can also define your own dynamic arrays by using the following
			structure:

			typedef struct {
				size_t capacity;
				size_t count;
				type items;	-- Type is whatever type is stored in this dynamic array
			} typename;	-- typename can be whatever you want

			There are 3 macros for manipulating dynamic arrays:
				ape_da_append(da, x)		-- Append x to da
				ape_da_append_many(da, xs, n)	-- Append n elements from xs to da
				ape_da_free(da)			-- Free da

			You can also change the count variable to anything you want,
			generally you should only ever set it to 0, so then the macros
			will actually assume it's empty and overwrite the data.
			Essentially just reusing the same memory for different things.

		ApeStrBuilder:
			ApeStrBuilder is a dynamic array of chars, it is very useful
			for building strings from smaller parts, there is a special utility
			function for this:
				ape_sb_append_str(sb, str)
				sb should be a pointer to an ApeStrBuilder
				str should be a null-terminated string
			If you want to append single characters you should use
			ape_da_append instead.
			The last character should always be NULL if you want to use
			the resulting string as an actual string
			The resulting string is is sb.items, where sb is an ApeStrBuilder

		ApeBuildTarget
			Exists just for convenience, it is an enum containing three
			different values:
				APE_TARGET_RELEASE,
				APE_TARGET_DEBUG,
				APE_TARGET_TEST,

	BOOTSTRAPPING YOUR BUILD SYSTEM
	===============================
	Instead of using the normal main function, you should use the custom
	APEBUILD_MAIN(int argc, char **argv) macro:

		APEBUILD_MAIN(int argc, char **argv) {
			// Your bootstrapper code goes here ...
		}

	Your bootstrapper will just be rebuilt first and then this function will be called.

	APEBUILD_MAIN should return one of:
		APEBUILD_SUCCESS
		APEBUILD_ERROR
	You can also call APEERROR(fmt, ...) to send a custom error message.

	Inside APEBUILD_MAIN you should use the following functions and macros:
		APE_BUILD_EXEC(name)
			Define an executable to be built

		APE_BUILD_LIB(name)
			Define a library to be built

		APE_BUILD_SOURCES_APPEND(name, ...)
			Add sources to library or executable, ... should be
			a comma separated list of file names

		APE_BUILD_LIBS_APPEND(name, ...)
			Add libraries to link with built library or executable,
			... should be a comma separated list of library names

		APE_BUILD_INCLUDES_APPEND(name, ...)
			Add include directories to library or executable,
			... should be a comma separated list of directory names

		ape_build_sources_append_dir(name, dirpath)
			Add all source files from directory at dirpath to library
			or executable.

		ape_build_target(name, target)
			Build the library or executable name,
			target should be one of:
				APE_TARGET_RELEASE
				APE_TARGET_DEBUG
				APE_TARGET_TEST

		ape_build_all_target(target)
			Build all libraries and executables,
			target should be one of:
				APE_TARGET_RELEASE
				APE_TARGET_DEBUG
				APE_TARGET_TEST

		ape_run(name, args)
			Run executable name with the arguments in args,
			args is a NULL terminated list of strings.
			Return value is 0 on success.

		ape_run_all()
			Run all executables, currently doesn't support any arguments

	You can also define tests:
		tests are just source files that are only included in APE_TARGET_TEST.
		When you build anything with APE_TARGET_TEST, the TEST macro is always
		defined by default.

		APE_BUILD_TESTS_APPEND(name, ...)
			Add tests to executable or library,
			... should be a comma separated list of file paths

		ape_build_tests_append_dir(name, dirpath)
			Add tests from directory to executable or library,
			dirpath should be a path to a directory

		ape_build_tests(name)
			Build name with target APE_TARGET_TEST

		ape_run_tests(name)
			Build name with target APE_TARGET_TEST and run it

		ape_run_tests_all()
			Build all executables and libraries containing test files
			with target APE_TARGET_TEST and run them


	If you want more control over what to build, you can use the following functions:

		char *ape_build_file(file, includes, target)
			file is a pointer to an ApeFile struct, it contains two values:
				filename: A string containing the file's full path
				type: The file type (extension), one of APE_FILE_C or APE_FILE_CXX

			includes is a pointer to an ApeStrList sruct, it's a dynamic
			array of strings containing the include paths.

			target is one of:
				APE_TARGET_RELEASE
				APE_TARGET_TEST
				APE_TARGET_DEBUG

			The return value is the full path of the resulting object file

		ApeStrList *ape_build_sources(files, includes, target)
			files is a pointer to an ApeFileList dynamic array containing
			ApeFile structs defining what files are to be built

			the return value is an ApeStrList dynamic array containing
			file paths to all of the resulting object files

			the rest are the same as ape_build_file

		ape_link_files(files, libs, outfname, target)
			files is a pointer to an ApeFileList dynamic array containing
			the object files to be linked

			libs is a pointer to an ApeStrList dynamic array containing
			the names of the libraries used

			outfname is a string containing the output executable or library name

			target is one of:
				APE_TARGET_RELEASE
				APE_TARGET_DEBUG
				APE_TARGET_TEST

	If that's still not enough control, you can use ApeCmd directly:
		ApeCmd is a dynamic array of strings, these strings are then converted
		to a command and it's arguments by ape_cmd_run_sync and ape_cmd_run_async

		ape_cmd_append(cmd, ...)
			cmd should be a pointer to an ApeCmd struct.
			... should be a comma separated list of arguments to append to cmd

		ape_cmd_run_sync(cmd)
			cmd should be an ApeCmd struct, not a pointer.
			This will run the command synchronously and return 0 on success

		ape_cmd_run_async(cmd)
			Same as ape_cmd_run_sync but run cmd asynchronously instead

		ape_cmd_free(cmd)
			Free cmd
			cmd should not be a pointer

	There are also functions for detecting when something needs to be rebuilt:
		These are automatically called by ape_build_*

		ape_needs_rebuild1(outfile, infile)
			outfile is a path to the file that should(maybe) be built
			infile is a path to the file to build from
			returns 1 if outfile needs to be rebuilt, otherwise, returns 0

		ape_needs_rebuild(outfile, infiles)
			outfile is a path to the file that should(maybe) be built
			infiles is an ApeStrList dynamic array containing all the files
			outfile should be built from
			returns 1 if outfile needs to be rebuilt, otherwise, returns 0

 */
#ifndef APEBUILD_IMPL
#pragma GCC error("Define APEBUILD_IMPL before including this")
#else /* ifdef APEBUILD_IMPL */

#ifndef APELD
#pragma GCC error("Define APELD (linker)")
#else /* ifdef APELD */

#ifdef APELANGC

#ifndef APELANGANY
#define APELANGANY
#endif /* APELANGANY */

#ifndef APECC
#pragma GCC error("Define APECC (c compiler)")
#else /* ifdef APECC */

#define APECFLAGS_INTERNAL_DEBUG "-DDEBUG"
#define APECFLAGS_INTERNAL "-c"
#define _APE_APEBUILD_VALID__

#ifdef APECFLAGS
#define _APECFLAGS APECFLAGS_INTERNAL, APECFLAGS
#else /* ifndef APECFLAGS */
#define _APECFLAGS APECFLAGS_INTERNAL
#endif /* APECFLAGS */

#ifdef APECFLAGS_DEBUG
#define _APECFLAGS_DEBUG APECFLAGS_INTERNAL_DEBUG, APECFLAGS_DEBUG
#else /* ifndef APECFLAGS_DEBUG */
#define _APECFLAGS_DEBUG APECFLAGS_INTERNAL_DEBUG
#endif /* APECFLAGS_DEBUG */

#endif /* APECC */

#endif /* APELANGC */

#ifdef APELANGCXX

#ifndef APELANGANY
#define APELANGANY
#endif /* APELANGANY */

#ifndef APECXX
#pragma GCC error("Define APECXX (c++ compiler)")
#else /* ifdef APECXX */

#define APECXXFLAGS_INTERNAL_DEBUG "-DDEBUG"
#define APECXXFLAGS_INTERNAL "-c"
#define _APE_APEBUILD_VALID__

#ifdef APECXXFLAGS
#define _APECXXFLAGS APECXXFLAGS, APECXXFLAGS_INTERNAL
#else /* ifndef APECXXFLAGS */
#define _APECXXFLAGS APECXXFLAGS_INTERNAL
#endif /* APECXXFLAGS */

#ifdef APECXXFLAGS_DEBUG
#define _APECXXFLAGS_DEBUG APECXXFLAGS_INTERNAL_DEBUG, APECXXFLAGS_DEBUG
#else /* ifndef APECXXFLAGS_DEBUG */
#define _APECXXFLAGS_DEBUG APECXXFLAGS_INTERNAL_DEBUG
#endif /* APECXXFLAGS_DEBUG */

#endif /* APECXX */

#endif /* APELANGCXX */

#ifndef APELANGANY
#pragma GCC error("Define one of APELANGC or APELANGCXX")
#endif /* APELANGANY */

#endif /* APELD */
#endif

#ifdef _APE_APEBUILD_VALID__
#ifndef _APEBUILD_H
#define _APEBUILD_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <wait.h>
#include <stdbool.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <dirent.h>
#else
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

struct dirent {
	char d_name[MAX_PATH + 1];
};

static DIR *opendir(const char *dirpath);
static struct dirent *readdir(DIR *dirp);
static int closedir(DIR *dirp);

#endif

#define BOLD "\x1b[1m"
#define FG_RED "\x1b[31m"
#define FG_GREEN "\x1b[32m"
#define FG_YELLOW "\x1b[33m"
#define FG_CYAN "\x1b[36m"
#define RESET "\x1b[0m"

#define eprintf(fmt, ...) fprintf(stderr, fmt __VA_OPT__(, ) __VA_ARGS__)

#define APE_LOG_INFO BOLD FG_GREEN "[INFO] " RESET
#define APE_LOG_DEBUG BOLD FG_CYAN "[DEBUG] " RESET
#define APE_LOG_WARN BOLD FG_YELLOW "[WARN] " RESET
#define APE_LOG_ERROR BOLD FG_RED "[ERROR] " RESET
#define APE_LOG(level, fmt, ...) \
	eprintf(level fmt "\n" __VA_OPT__(, ) __VA_ARGS__)
#define APEINFO(fmt, ...) APE_LOG(APE_LOG_INFO, fmt __VA_OPT__(, ) __VA_ARGS__)
#define APEDEBUG(fmt, ...) \
	APE_LOG(APE_LOG_DEBUG, fmt __VA_OPT__(, ) __VA_ARGS__)
#define APEWARN(fmt, ...) APE_LOG(APE_LOG_WARN, fmt __VA_OPT__(, ) __VA_ARGS__)
#define APEERROR(fmt, ...) \
	APE_LOG(APE_LOG_ERROR, fmt __VA_OPT__(, ) __VA_ARGS__)

typedef enum {
	APE_TARGET_RELEASE,
	APE_TARGET_DEBUG,
	APE_TARGET_TEST,
} ApeBuildTarget;

int ape_endswith(char *s, const char *suffix)
{
	if (!s || !suffix)
		return 0;
	size_t lens = strlen(s);
	size_t lensuf = strlen(suffix);
	if (lensuf > lens)
		return 0;
	return strncmp(s + lens - lensuf, suffix, lensuf) == 0;
}

#define APE_DA_INIT_CAP 256

#define ape_da_append(da, x)                                              \
	do {                                                              \
		if ((da)->count + 1 >= (da)->capacity) {                  \
			(da)->capacity = (da)->capacity == 0 ?            \
						 APE_DA_INIT_CAP :        \
						 (da)->capacity * 2;      \
			(da)->items = (typeof((da)->items))realloc(       \
				(da)->items,                              \
				(da)->capacity * sizeof((da)->items[0])); \
		}                                                         \
		(da)->items[(da)->count++] = x;                           \
	} while (0)

#define ape_da_append_many(da, xs, n)                                        \
	do {                                                                 \
		if ((da)->count + (n) > (da)->capacity) {                    \
			if ((da)->capacity == 0) {                           \
				(da)->capacity = APE_DA_INIT_CAP;            \
			}                                                    \
			while ((da)->count + (n) > (da)->capacity) {         \
				(da)->capacity *= 2;                         \
			}                                                    \
			(da)->items = realloc((da)->items,                   \
					      (da)->capacity *               \
						      sizeof(*(da)->items)); \
		}                                                            \
		memcpy((da)->items + (da)->count, (xs),                      \
		       (n) * sizeof(*(da)->items));                          \
		(da)->count += (n);                                          \
	} while (0)

#define ape_da_free(da) free(da.items)

typedef struct {
	size_t capacity;
	size_t count;
	const char **items;
} ApeCmd;

#define ape_cmd_append(cmd, ...)                                      \
	ape_da_append_many(cmd, ((const char *[]){ __VA_ARGS__ }),    \
			   (sizeof((const char *[]){ __VA_ARGS__ }) / \
			    sizeof(const char *)))

#define ape_cmd_free(cmd) free(cmd.items)

typedef struct {
	size_t capacity;
	size_t count;
	char *items;
} ApeStrBuilder;

#define ape_sb_append_str(sb, str)            \
	do {                                  \
		const char *s = (str);        \
		size_t n = strlen(s);         \
		ape_da_append_many(sb, s, n); \
	} while (0)

void ape_cmd_render(ApeCmd cmd, ApeStrBuilder *render)
{
	for (size_t i = 0; i < cmd.count; i++) {
		const char *arg = cmd.items[i];
		if (arg == NULL)
			break;
		if (i > 0)
			ape_sb_append_str(render, " ");
		if (!strchr(arg, ' ')) {
			ape_sb_append_str(render, arg);
		} else {
			ape_da_append(render, '\'');
			ape_sb_append_str(render, arg);
			ape_da_append(render, '\'');
		}
	}
}

#ifdef _WIN32
typedef HANDLE ApeProc;
#define APE_INVALID_PROC INVALID_HANDLE_VALUE
#else
typedef int ApeProc;
#define APE_INVALID_PROC (-1)
#endif

ApeProc ape_run_cmd_async(ApeCmd cmd)
{
	if (cmd.count < 1) {
		APEERROR("Can't execute empty command");
		return APE_INVALID_PROC;
	}
	ApeStrBuilder sb = { 0 };
	ape_cmd_render(cmd, &sb);
	ape_da_append(&sb, '\0');
	APEINFO("CMD: %s", sb.items);
	ape_da_free(sb);
	memset(&sb, 0, sizeof(sb));

#ifdef _WIN32
	STARTUPINFO siStartInfo;
	ZeroMemory(&siStartInfo, sizeof(siStartInfo));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES

		PROCESS_INFORMATION piProcInfo;
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	ape_cmd_render(cmd, &sb);
	ape_da_append(&sb, '\0');
	BOOL bSuccess = CreateProcessA(NULL, sb.items, NULL, NULL, TRUE, 0,
				       NULL, NULL, &siStartInfo, &piProcInfo);
	ape_da_free(sb);

	if (!bSuccess) {
		APEERROR("Could not create child process: %lu", GetLastError());
		return APE_INVALID_PROC;
	}

	CloseHandle(piProcInfo.hThread);
	return piProcInfo.hProcess;
#else

	pid_t cpid = fork();
	if (cpid < 0) {
		APEERROR("Could not fork child process: %s", strerror(errno));
		return APE_INVALID_PROC;
	}
	if (cpid == 0) {
		ApeCmd cmd_null = { 0 };
		ape_da_append_many(&cmd_null, cmd.items, cmd.count);
		ape_cmd_append(&cmd_null, NULL);
		if (execvp(cmd.items[0], (char *const *)cmd_null.items) < 0) {
			APEERROR("Could not exec child process: %s",
				 strerror(errno));
			exit(1);
		}
		assert(0 && "Ureachable");
	}
	return cpid;
#endif
}

bool ape_proc_wait(int proc)
{
	if (proc == -1)
		return false;
#ifdef _WIN32
	DWORD result = WaitForSingleObject(proc, INFINITE);
	if (result == WAIT_FAILED) {
		APEERROR("Could not wait on child process: %lu",
			 GetLastError());
		return false;
	}

	DWORD exit_status : if (!GetExitCodeProcess(proc, &exit_status))
	{
		APEERROR("Could not get process exit code: %lu",
			 GetLastError());
		return false;
	}

	if (exit_status != 0) {
		APEERROR("Command exited with exit code: %lu", exit_status);
		return false;
	}

	CloseHandle(proc);

	return true;
#else

	for (;;) {
		int wstatus = 0;
		if (waitpid(proc, &wstatus, 0) < 0) {
			APEERROR("Could not wait on command (pid %d): %s", proc,
				 strerror(errno));
			return false;
		}
		if (WIFEXITED(wstatus)) {
			int exit_status = WEXITSTATUS(wstatus);
			if (exit_status != 0) {
				APEERROR("Command exited with error code: %d",
					 exit_status);
				return false;
			}
			break;
		}
		if (WIFSIGNALED(wstatus)) {
			APEERROR("Command was terminated by %s",
				 strsignal(WTERMSIG(wstatus)));
			return false;
		}
	}
	return true;
#endif
}

bool ape_cmd_run_sync(ApeCmd cmd)
{
	int p = ape_run_cmd_async(cmd);
	if (p == -1)
		return false;
	return ape_proc_wait(p);
}

#define APE_FILE_C 1
#define APE_FILE_CXX 2

typedef struct {
	int type;
	char *filename;
} ApeFile;

typedef struct {
	size_t capacity;
	size_t count;
	ApeFile *items;
} ApeFileList;

typedef enum {
	APE_BUILD_EXEC,
	APE_BUILD_LIB,
} ApeBuildType;

typedef struct {
	size_t capacity;
	size_t count;
	char **items;
} ApeStrList;

typedef struct {
	char *outname;
	ApeFileList infiles;
	ApeFileList testfiles;
	ApeBuildType type;
	ApeStrList libs;
	ApeStrList includes;
} ApeBuild;

static struct {
	size_t capacity;
	size_t count;
	ApeBuild *items;
} ApeBuilds = { 0 };

ApeBuild *ape_find_build(char *name)
{
	for (size_t i = 0; i < ApeBuilds.count; i++) {
		if (strcmp(ApeBuilds.items[i].outname, name) == 0)
			return &ApeBuilds.items[i];
	}
	return NULL;
}

#define APE_BUILD_SOURCES_APPEND(name, ...)                           \
	do {                                                          \
		ApeBuild *ab = ape_find_build(name);                  \
		if (ab == NULL) {                                     \
			APEERROR("Build target not found: %s", name); \
			break;                                        \
		}                                                     \
		char *__flist[] = { __VA_ARGS__, NULL };              \
		for (size_t i = 0; __flist[i] != NULL; i++) {         \
			ApeFile f = { 0 };                            \
			if (ape_endswith(__flist[i], ".c"))           \
				f.type = APE_FILE_C;                  \
			if (ape_endswith(__flist[i], ".cpp"))         \
				f.type = APE_FILE_CXX;                \
			f.filename = __flist[i];                      \
			ape_da_append(&(ab->infiles), f);             \
		}                                                     \
	} while (0)

#define APE_BUILD_TESTS_APPEND(name, ...)                             \
	do {                                                          \
		ApeBuild *ab = ape_find_build(name);                  \
		if (ab == NULL) {                                     \
			APEERROR("Build target not found: %s", name); \
			break;                                        \
		}                                                     \
		char *__flist[] = { __VA_ARGS__, NULL };              \
		for (size_t i = 0; __flist[i] != NULL; i++) {         \
			ApeFile f = { 0 };                            \
			if (ape_endswith(__flist[i], ".c"))           \
				f.type = APE_FILE_C;                  \
			if (ape_endswith(__flist[i], ".cpp"))         \
				f.type = APE_FILE_CXX;                \
			f.filename = __flist[i];                      \
			ape_da_append(&(ab->testfiles), f);           \
		}                                                     \
	} while (0)

int ape_build_sources_append_dir(char *name, char *path)
{
	DIR *dir = opendir(path);
	struct dirent *entry;
	if (!dir) {
		APEERROR("Could not open directory %s", path);
		return 1;
	}
	while ((entry = readdir(dir)) != NULL) {
		ApeStrBuilder f = { 0 };
		if (entry->d_name[0] == '.')
			continue;
		ape_sb_append_str(&f, path);
		if (f.items[f.count - 1] != '/')
			ape_da_append(&f, '/');
		ape_sb_append_str(&f, entry->d_name);
		ape_da_append(&f, '\0');
		struct stat statbuf;
		if (stat(f.items, &statbuf) < 0) {
			APEERROR("Could not get stat of %s: %s", f.items,
				 strerror(errno));
			return -1;
		}
		switch (statbuf.st_mode & S_IFMT) {
		case S_IFDIR:
			ape_build_sources_append_dir(name, f.items);
			break;
		case S_IFREG:
			APE_BUILD_SOURCES_APPEND(name, f.items);
			break;
		default:
			break;
		}
	}
}

int ape_build_tests_append_dir(char *name, char *path)
{
	DIR *dir = opendir(path);
	struct dirent *entry;
	if (!dir) {
		APEERROR("Could not open directory %s", path);
		return 1;
	}
	while ((entry = readdir(dir)) != NULL) {
		ApeStrBuilder f = { 0 };
		if (entry->d_name[0] == '.')
			continue;
		ape_sb_append_str(&f, path);
		if (f.items[f.count - 1] != '/')
			ape_da_append(&f, '/');
		ape_sb_append_str(&f, entry->d_name);
		ape_da_append(&f, '\0');
		struct stat statbuf;
		if (stat(f.items, &statbuf) < 0) {
			APEERROR("Could not get stat of %s: %s", f.items,
				 strerror(errno));
			return -1;
		}
		switch (statbuf.st_mode & S_IFMT) {
		case S_IFDIR:
			ape_build_tests_append_dir(name, f.items);
			break;
		case S_IFREG:
			APE_BUILD_TESTS_APPEND(name, f.items);
			break;
		default:
			break;
		}
	}
}

#define APE_BUILD_LIBS_APPEND(name, ...)                                       \
	do {                                                                   \
		ApeBuild *ab = ape_find_build(name);                           \
		if (ab == NULL) {                                              \
			APEERROR("Build target not found: %s", name);          \
			break;                                                 \
		}                                                              \
		ape_da_append_many(ab->libs,                                   \
				   ((const char *[]){ __VA_ARGS__ }),          \
				   (sizeof((const char *[]){ __VA_ARGS__ }))); \
	} while (0)

#define APE_BUILD_INCLUDES_APPEND(name, incl)                         \
	do {                                                          \
		ApeBuild *ab = ape_find_build(name);                  \
		if (ab == NULL) {                                     \
			APEERROR("Build target not found: %s", name); \
			break;                                        \
		}                                                     \
		ape_da_append(&(ab->includes), incl);                 \
	} while (0)

#define APE_BUILD_EXEC(name)                                           \
	ape_da_append(&ApeBuilds, ((ApeBuild){ .outname = name,        \
					       .infiles = { 0 },       \
					       .type = APE_BUILD_EXEC, \
					       .libs = { 0 },          \
					       .includes = { 0 } }))

#define APE_BUILD_LIB(name)                                          \
	ape_da_append(&ApeBuilds, (ApeBuild){ .outname = name,       \
					      .infiles = { 0 },      \
					      .type = APE_BUILD_LIB, \
					      .libs = { 0 },         \
					      .includes = { 0 } })

int ape_needs_rebuild(const char *outfile, ApeStrList *infiles)
{
#ifdef _WIN32
	BOOL bSuccess;
	HANDLE outpath_fd = CreateFile(outfile, GENERIC_READ, 0, NULL,
				       OPEN_EXISTING, FILE_ATTRIBUTE_READONLY,
				       NULL);
	if (outpath_fd == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			return 1;
		APEERROR("Could not open file %s: %lu", outfile,
			 GetLastError());
		return -1;
	}
	FILETIME outpath_time;
	bSuccess = GetFileTime(outfile, NULL, NULL, &outpath_time);
	CloseHandle(outpath_fd);
	if (!bSuccess) {
		APEERROR("Could not get time of %s: %lu", outfile,
			 GetLastError());
		return -1;
	}
	for (size_t i = 0; i < infiles->count; i++) {
		const char *infile = infiles->items[i];
		HANDLE inpath_fd = CreateFile(infile, GENERIC_READ, 0, NULL,
					      OPEN_EXISTING,
					      FILE_ATTRIBUTE_READONLY, NULL);
		if (inpath_fd == INVALID_HANDLE_VALUE) {
			APEERROR("Could not open file %s: %lu", infile,
				 GetLastError());
			return -1;
		}
		FILETIME inpath_time;
		bSuccess = GetFileTime(inpath_fd, NULL, NULL, &inpath_time);
		CloseHandle(inpath_fd);
		if (!bSuccess) {
			APEERROR("Could not get time of %s: %lu", infile,
				 GetLastError());
			return -1;
		}
		if (CompareFileTime(&inpath_time, &outpath_time) == 1)
			return 1;
	}
	return 0;
#else
	struct stat outs;
	if (stat(outfile, &outs) != 0) {
		return 1;
	}
	struct stat ins;
	for (size_t i = 0; i < infiles->count; i++) {
		if (stat(infiles->items[i], &ins) != 0) {
			APEERROR("Failed to get stat (of file %s): %s",
				 infiles->items[i], strerror(errno));
			return 1;
		}
		if (ins.st_mtime > outs.st_mtime) {
			return 1;
		}
	}
	return 0;
#endif
}

int ape_needs_rebuild1(const char *outfile, const char *infile)
{
	ApeStrList s = { 0 };
	ape_da_append(&s, (char*)infile);
	return ape_needs_rebuild(outfile, &s);
}

char *ape_build_file(ApeFile *file, ApeStrList *includes, ApeBuildTarget target)
{
	ApeStrBuilder outfname = { 0 };
	ape_sb_append_str(&outfname, strdup(file->filename));
	ape_sb_append_str(&outfname, ".o");
	ape_da_append(&outfname, '\0');
	if (!ape_needs_rebuild1(outfname.items, file->filename))
		return outfname.items;
	ApeCmd cmd = { 0 };
#ifdef APELANGC
	if (file->type == APE_FILE_C) {
		ape_cmd_append(&cmd, APECC);
#ifdef _APECFLAGS
		ape_cmd_append(&cmd, _APECFLAGS);
#endif
#ifdef _APECFLAGS_DEBUG
		if (target == APE_TARGET_DEBUG || target == APE_TARGET_TEST) {
			ape_cmd_append(&cmd, _APECFLAGS_DEBUG);
		}
#endif
		if (target == APE_TARGET_TEST) {
			ape_cmd_append(&cmd, "-DTEST");
		}
	}
#endif
#ifdef APELANGCXX
	if (file->type == APE_FILE_CXX) {
		ape_cmd_append(&cmd, APECXX);
#ifdef _APECXXFLAGS
		ape_cmd_append(&cmd, _APECXXFLAGS);
#endif
#ifdef _APECXXFLAGS_DEBUG
		if (target == APE_TARGET_DEBUG || target == APE_TARGET_TEST) {
			ape_cmd_append(&cmd, _APECXXFLAGS_DEBUG);
		}
#endif
		if (target == APE_TARGET_TEST) {
			ape_cmd_append(&cmd, "-DTEST");
		}
	}
#endif
	if (file->type == 0)
		return NULL;
	if (includes->count > 0) {
		for (size_t i = 0; i < includes->count; i++) {
			ApeStrBuilder I = { 0 };
			ape_sb_append_str(&I, "-I");
			ape_sb_append_str(&I, strdup(includes->items[i]));
			ape_da_append(&I, '\0');
			ape_cmd_append(&cmd, I.items);
		}
	}
	ape_cmd_append(&cmd, file->filename);
	ape_cmd_append(&cmd, "-o");
	ape_cmd_append(&cmd, outfname.items);
	ape_cmd_run_sync(cmd);
	return outfname.items;
}

ApeStrList ape_build_sources(ApeFileList *files, ApeStrList *includes,
			     ApeBuildTarget target)
{
	ApeStrList out = { 0 };
	for (size_t i = 0; i < files->count; i++) {
		char *ob = ape_build_file(&(files->items[i]), includes, target);
		if (ob != NULL)
			ape_da_append(&out, ob);
	}
	return out;
}

void ape_link_files(ApeStrList *files, ApeStrList *libs, char *outfname,
		    ApeBuildTarget target)
{
	ApeCmd cmd = { 0 };
	ape_cmd_append(&cmd, APELD);
	ape_cmd_append(&cmd, "-o", outfname);
	if (libs->count > 0) {
		for (size_t i = 0; i < libs->count; i++) {
			ApeStrBuilder l = { 0 };
			ape_sb_append_str(&l, "-l");
			ape_sb_append_str(&l, strdup(libs->items[i]));
			ape_da_append(&l, '\0');
			ape_cmd_append(&cmd, l.items);
		}
	}
	ape_da_append_many(&cmd, files->items, files->count);
	ape_cmd_run_sync(cmd);
}

void ape_build_target(char *name, ApeBuildTarget target)
{
	ApeBuild *ab = ape_find_build(name);
	if (ab == NULL) {
		APEERROR("Couldn't build target %s", name);
		exit(1);
	}
	ApeStrList obj =
		ape_build_sources(&(ab->infiles), &(ab->includes), target);
	if (obj.count > 0 && ape_needs_rebuild(ab->outname, &obj)) {
		ape_link_files(&obj, &(ab->libs), ab->outname, target);
	} else {
		APEINFO("No files to build");
	}
}

void ape_build_all_target(ApeBuildTarget target)
{
	for (size_t i = 0; i < ApeBuilds.count; i++) {
		ApeStrList obj = ape_build_sources(
			&(ApeBuilds.items[i].infiles),
			&(ApeBuilds.items[i].includes), target);
		if (obj.count > 0 &&
		    ape_needs_rebuild(ApeBuilds.items[i].outname, &obj)) {
			ape_link_files(&obj, &(ApeBuilds.items[i].libs),
				       ApeBuilds.items[i].outname, target);
		} else {
			APEINFO("No files to build");
		}
	}
}

void ape_build_tests(char *name)
{
	ApeBuild *ab = ape_find_build(name);
	if (ab == NULL) {
		APEERROR("Couldn't build target %s", name);
		exit(1);
	}
	ApeStrList obj = ape_build_sources(&(ab->infiles), &(ab->includes),
					   APE_TARGET_TEST);
	ApeStrList tobj = ape_build_sources(&(ab->testfiles), &(ab->includes),
					    APE_TARGET_TEST);
	ape_da_append_many(&obj, tobj.items, tobj.count);
	if (obj.count > 0 && ape_needs_rebuild(ab->outname, &obj)) {
		ape_link_files(&obj, &(ab->libs), ab->outname, APE_TARGET_TEST);
	} else {
		APEINFO("No files to build");
	}
}

int ape_run(char *name, char **args)
{
	ApeBuild *ab = ape_find_build(name);
	if (ab == NULL) {
		APEERROR("Couldn't run target %s", name);
		exit(1);
	}
	ApeCmd cmd = { 0 };
	ApeStrBuilder sb = { 0 };
	ape_sb_append_str(&sb, "./");
	ape_sb_append_str(&sb, ab->outname);
	ape_cmd_append(&cmd, sb.items);
	char **a = args;
	while ((*a) != NULL) {
		ape_cmd_append(&cmd, (*a));
		a++;
	}
	return ape_cmd_run_sync(cmd);
}

void ape_run_all()
{
	for (size_t i = 0; i < ApeBuilds.count; i++) {
		ApeCmd cmd = { 0 };
		ApeStrBuilder sb = { 0 };
		ape_sb_append_str(&sb, "./");
		ape_sb_append_str(&sb, ApeBuilds.items[i].outname);
		ape_cmd_append(&cmd, sb.items);
		ape_cmd_run_sync(cmd);
	}
}

void ape_run_tests(char *name)
{
	ApeBuild *ab = ape_find_build(name);
	if (ab == NULL) {
		APEERROR("Couldn't run target %s", name);
		exit(1);
	}
	if (ab->testfiles.count <= 0) {
		APEERROR("Target %s doesn't contain tests", name);
		exit(1);
	}
	ape_build_tests(name);
	ApeCmd cmd = { 0 };
	ApeStrBuilder sb = { 0 };
	ape_sb_append_str(&sb, "./");
	ape_sb_append_str(&sb, ab->outname);
	ape_cmd_append(&cmd, sb.items);
	ape_cmd_run_sync(cmd);
}

void ape_run_tests_all()
{
	for (size_t i = 0; i < ApeBuilds.count; i++) {
		if (ApeBuilds.items[i].testfiles.count <= 0)
			continue;
		ape_build_tests(ApeBuilds.items[i].outname);
		ApeCmd cmd = { 0 };
		ApeStrBuilder sb = { 0 };
		ape_sb_append_str(&sb, "./");
		ape_sb_append_str(&sb, ApeBuilds.items[i].outname);
		ape_cmd_run_sync(cmd);
	}
}

int ape_rename(const char *oldname, const char *newname)
{
	if (rename(oldname, newname) == 0) {
		APEINFO("%s > %s", oldname, newname);
		return 1;
	}
	APEERROR("Couldn't rename file %s: %s", oldname, strerror(errno));
	return 0;
}

#define _APE_REBUILD(binpath, srcpath) "gcc", "-o", binpath, srcpath

#define APE_REBUILD(argc, argv)                                            \
	do {                                                               \
		const char *srcpath = __FILE__;                            \
		assert(argc >= 1);                                         \
		const char *binpath = argv[0];                             \
		int rebuild_needed = ape_needs_rebuild1(binpath, srcpath); \
		if (rebuild_needed < 0)                                    \
			exit(1);                                           \
		if (rebuild_needed) {                                      \
			ApeStrBuilder sb = { 0 };                          \
			ape_sb_append_str(&sb, binpath);                   \
			ape_sb_append_str(&sb, ".old");                    \
			ape_da_append(&sb, '\0');                          \
			if (!ape_rename(binpath, sb.items))                \
				exit(1);                                   \
			ApeCmd rebuild = { 0 };                            \
			ape_cmd_append(&rebuild,                           \
				       _APE_REBUILD(binpath, srcpath));    \
			bool rebuilt = ape_cmd_run_sync(rebuild);          \
			if (!rebuilt) {                                    \
				ape_rename(sb.items, binpath);             \
				exit(1);                                   \
			}                                                  \
			ApeCmd cmd = { 0 };                                \
			ape_da_append_many(&cmd, argv, argc);              \
			if (!ape_cmd_run_sync(cmd))                        \
				exit(1);                                   \
			exit(0);                                           \
		}                                                          \
	} while (0)

#define APEBUILD_MAIN(a_argc, a_argv)             \
	int apebuild_main(a_argc, a_argv);        \
	int main(int argc, char **argv)           \
	{                                         \
		APE_REBUILD(argc, argv);          \
		return apebuild_main(argc, argv); \
	}                                         \
	int apebuild_main(a_argc, a_argv)

#ifdef _WIN32

struct DIR {
	HANDLE hFind;
	WIN32_FIND_DATA data;
	struct dirent *dirent;
};

struct dirent {
	char d_name[MAX_PATH + 1];
};

DIR *opendir(const char *dirpath)
{
	assert(dirpath);
	char buffer[MAX_PATH];
	snprintf(buffer, MAX_PATH, "%s\\*", dirpath);
	DIR *dir = (DIR *)calloc(1, sizeof(DIR));
	dir->hFind = FindFirstFile(buffer, &dir->data);
	if (dir->hFind == INVALID_HANDLE_VALUE) {
		errno = ENOSYS;
		goto fail;
	}
	return dir;
	if (dir) {
		free(dir);
	}
	return NULL;
}

struct dirent *readdir(DIR *dirp)
{
	assert(dirp);
	if (dirp->dirent == NULL) {
		dirp->dirent = (struct dirent *)calloc(1, sizeof(dirent));
	} else {
		if (!FindNextFile(dirp->hFind, &dirp->data)) {
			if (GetLastError() != ERROR_NO_MORE_FILES) {
				errno = ENOSYS;
			}
			return NULL;
		}
	}
	memset(dirp->dirent->d_name, 0, sizeof(dirp->dirent->d_name));
	strncpy(dirp->dirent->d_name, dirp->data.cFileName,
		sizeof(dirp->dirent->d_name) - 1);
	return dirp->dirent;
}

int closedir(DIR *dirp)
{
	assert(dirp);
	if (!FindClose(dirp->hFind)) {
		errno = ENOSYS;
		return -1;
	}
	if (dirp->dirent) {
		free(dirp->dirent);
	}
	free(dirp);
	return 0;
}

#endif

#endif
#endif
