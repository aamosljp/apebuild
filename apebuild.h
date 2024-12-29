#define __STDC_WANT_LIB_EXT1__ 1
#include <dirent.h>
#include <sys/stat.h>
#include <wait.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
	size_t capacity;
	size_t count;
	char *items;
} ApeStrBuilder;

typedef struct {
	size_t capacity;
	size_t count;
	char **items;
} ApeStrList;

typedef struct {
	size_t capacity;
	size_t count;
	const char **items;
} ApeCmd;

typedef struct {
	size_t capacity;
	size_t count;
	ApeCmd *items;
} ApeCmdList;

typedef int ApeProc;
#define APE_INVALID_PROC (-1)

void ape_cmd_render(ApeCmd cmd, ApeStrBuilder *render);
ApeProc ape_run_cmd_async(ApeCmd cmd);
int ape_proc_wait(int proc);
int ape_cmd_run_sync(ApeCmd cmd);
int ape_cmds_run(ApeCmdList cmds);

int ape_rename(const char *oldname, const char *newname);
char *ape_objfile_name(char *srcfilename);
int ape_needs_rebuild(const char *outfile, char **infiles, size_t len);
int ape_needs_rebuild1(const char *outfile, const char *infile);
int ape_endswith(char *s, const char *suffix);

typedef struct {
	struct {
		size_t capacity;
		size_t count;
		char **items;
	} infiles;
	char *outfile;
	uint16_t flags;
	ApeStrList extra_build_args;
	ApeStrList extra_link_args;
} ApeBuilder;

ApeCmd ape_gen_build_command(char *srcfilename, uint16_t flags,
			     ApeStrList args);
ApeCmd ape_gen_link_command(char *outfilename, char **srcfilenames, size_t len,
			    uint16_t flags, ApeStrList args);
ApeCmdList ape_builder_gen_commands(ApeBuilder *builder);
void ape_builder_append_file(ApeBuilder *builder, char *path);
int ape_builder_append_dir(ApeBuilder *builder, char *path);
int ape_builder_append_dir_recursive(ApeBuilder *builder, char *path);
int ape_run_builder(ApeBuilder *builder);

int ape_run(int argc, char **argv);

#ifndef APE_OUTPUT_DIR
#define APE_OUTPUT_DIR "build/"
#endif
#define APE__OUTPUT_DIR(file) APE_OUTPUT_DIR file

#ifndef APE_REBUILD_COMMAND
#define APE_REBUILD_COMMAND(out, in) "gcc", "-o", out, in
#endif

#ifdef APE_PRESET_LINUX_GCC_C
#define APECC "gcc"
#define APELD "gcc"
#define APE_OBJ_EXTENSION ".o"
#define APE_SRC_EXTENSION ".c"
#define APE_BUILD_SRC_ARGS(infile, outfile) "-c", infile, "-o", outfile
#define APE_LINK_ARGS(outfile) "-o", outfile
#define APE_LINK_ARGS_ADD_LIB(lib) "-l" lib
#define APE_LINK_ARGS_ADD_LIBDIR(dir) "-L" dir
#define APE_LINK_ARGS_SHARED_LIB(outfile) "-shared", "-fPIC", "-o", outfile
#define APE_LIB_PREFIX "lib"
#define APE_LIB_SUFFIX ".so"
#endif

#ifdef APE_PRESET_LINUX_GCC_CXX
#define APECC "g++"
#define APELD "g++"
#define APE_OBJ_EXTENSION ".o"
#define APE_SRC_EXTENSION ".cpp"
#define APE_BUILD_SRC_ARGS(infile, outfile) "-c", infile, "-o", outfile
#define APE_LINK_ARGS(outfile) "-o", outfile
#define APE_LINK_ARGS_ADD_LIB(lib) "-l" lib
#define APE_LINK_ARGS_ADD_LIBDIR(dir) "-L" dir
#define APE_LINK_ARGS_SHARED_LIB(outfile) "-shared", "-fPIC", "-o", outfile
#define APE_LIB_PREFIX "lib"
#define APE_LIB_SUFFIX ".so"
#endif

#define APE_SET_FLAG(flag) (ape__builder.flags |= (1 << flag))
enum ApeFlag {
	APE_FLAG_REBUILD,
	APE_FLAG_SHARED_LIB,
};

#define APE_BUILDER(name, input)                                           \
	do {                                                               \
		ApeBuilder ape__builder = (ApeBuilder){ .outfile = name }; \
		{ input } ape_da_append(&ape__builder_list, ape__builder); \
	} while (0);

#define APE_INPUT_DIR(path) ape_builder_append_dir(&ape__builder, path)
#define APE_INPUT_DIR_REC(path) \
	ape_builder_append_dir_recursive(&ape__builder, path)
#define APE_INPUT_FILE(path) ape_builder_append_file(&ape__builder, path)
#define APE_INCLUDE_DIR(path) \
	ape_da_append(&ape__builder.extra_build_args, "-I" path)
#define APE_ADD_LIB(path)                            \
	ape_da_append(&ape__builder.extra_link_args, \
		      APE_LINK_ARGS_ADD_LIB(path))
#define APE_ADD_LIBDIR(path)                         \
	ape_da_append(&ape__builder.extra_link_args, \
		      APE_LINK_ARGS_ADD_LIBDIR(path))

#define APE_REBUILD(argc, argv)                                                \
	do {                                                                   \
		const char *srcpath = __FILE__;                                \
		assert(argc >= 1);                                             \
		const char *binpath = argv[0];                                 \
		int rebuild_needed = ape_needs_rebuild1(binpath, srcpath);     \
		if (rebuild_needed < 0)                                        \
			exit(1);                                               \
		if (rebuild_needed) {                                          \
			ApeStrBuilder sb = { 0 };                              \
			ape_sb_append_str(&sb, binpath);                       \
			ape_sb_append_str(&sb, ".old");                        \
			ape_da_append(&sb, '\0');                              \
			if (!ape_rename(binpath, sb.items))                    \
				exit(1);                                       \
			ApeCmd rebuild = { 0 };                                \
			ape_cmd_append(&rebuild,                               \
				       APE_REBUILD_COMMAND(binpath, srcpath)); \
			int rebuilt = ape_cmd_run_sync(rebuild);               \
			if (!rebuilt) {                                        \
				ape_rename(sb.items, binpath);                 \
				exit(1);                                       \
			}                                                      \
			ApeCmd cmd = { 0 };                                    \
			ape_da_append_many(&cmd, argv, argc);                  \
			if (!ape_cmd_run_sync(cmd))                            \
				exit(1);                                       \
			exit(0);                                               \
		}                                                              \
	} while (0)

#define APEBUILD_MAIN(...)                        \
	int apebuild_main(int argc, char **argv); \
	int main(int argc, char **argv)           \
	{                                         \
		APE_REBUILD(argc, argv);          \
		return apebuild_main(argc, argv); \
	}                                         \
	int apebuild_main(int argc, char **argv)

#ifndef APEBUILD_IMPLEMENTATION
#pragma GCC error("Define APEBUILD_IMPLEMENTATION before including apebuild.h")
#else

#ifndef APECC
#pragma GCC error "Define APECC as your compiler"
#endif
#ifndef APELD
#pragma GCC error "Define APELD as your linker"
#endif
#ifndef APE_SRC_EXTENSION
#pragma GCC error "Define APE_SRC_EXTENSION as your source file extension"
#endif
#ifndef APE_OBJ_EXTENSION
#pragma GCC error \
	"Define APE_OBJ_EXTENSION as your object file extension (appended to source file name)"
#endif
#ifndef APE_BUILD_SRC_ARGS
#pragma GCC error("Define APE_BUILD_SRC_ARGS(infile, outfile)")
#endif
#ifndef APE_LINK_ARGS
#pragma GCC error("Define APE_LINK_ARGS(outfile)")
#endif

#ifndef APE_LINK_ARGS_ADD_LIB
#pragma GCC warning \
	"If you have library dependencies, you should define APE_LINK_ARGS_ADD_LIB(lib)"
#endif

#ifndef APE_LINK_ARGS_ADD_LIBDIR
#pragma GCC warning \
	"If you have a custom library directory, you should define APE_LINK_ARGS_ADD_LIBDIR(dir)"
#endif

#ifndef APE_LINK_ARGS_SHARED_LIB
#pragma GCC warning \
	"If you are building libraries, you should define APE_LINK_ARGS_SHARED_LIB"
#endif

/*************************
 * Dynamic Array Start
 *************************/

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

#define ape_sb_append_str(sb, str)            \
	do {                                  \
		const char *s = (str);        \
		size_t n = strlen(s);         \
		ape_da_append_many(sb, s, n); \
	} while (0)

/*************************
 * Dynamic Array End
 *************************/

/*************************
 * Command Utilities Start
 *************************/

#define ape_cmd_append(cmd, ...)                                      \
	ape_da_append_many(cmd, ((const char *[]){ __VA_ARGS__ }),    \
			   (sizeof((const char *[]){ __VA_ARGS__ }) / \
			    sizeof(const char *)))

#define ape_cmd_free(cmd) free(cmd.items)

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

ApeProc ape_run_cmd_async(ApeCmd cmd)
{
	if (cmd.count < 1) {
		fprintf(stderr, "ERROR: Can't execute empty command\n");
		return APE_INVALID_PROC;
	}
	ApeStrBuilder sb = { 0 };
	ape_cmd_render(cmd, &sb);
	ape_da_append(&sb, '\0');
	fprintf(stderr, "CMD: %s\n", sb.items);
	ape_da_free(sb);
	memset(&sb, 0, sizeof(sb));

	pid_t cpid = fork();
	if (cpid < 0) {
		fprintf(stderr, "ERROR: Could not fork child process: %s\n",
			strerror(errno));
		return APE_INVALID_PROC;
	}
	if (cpid == 0) {
		ApeCmd cmd_null = { 0 };
		ape_da_append_many(&cmd_null, cmd.items, cmd.count);
		ape_cmd_append(&cmd_null, NULL);
		if (execvp(cmd.items[0], (char *const *)cmd_null.items) < 0) {
			fprintf(stderr,
				"ERROR: Could not exec child process: %s\n",
				strerror(errno));
			exit(1);
		}
		assert(0 && "Ureachable");
	}
	return cpid;
}

int ape_proc_wait(int proc)
{
	if (proc == -1)
		return 0;
	for (;;) {
		int wstatus = 0;
		if (waitpid(proc, &wstatus, 0) < 0) {
			fprintf(stderr,
				"ERROR: Could not wait on command (pid "
				"%d): %s\n",
				proc, strerror(errno));
			return 0;
		}
		if (WIFEXITED(wstatus)) {
			int exit_status = WEXITSTATUS(wstatus);
			if (exit_status != 0) {
				fprintf(stderr,
					"ERROR: Command exited with "
					"error code: %d\n",
					exit_status);
				return 0;
			}
			break;
		}
		if (WIFSIGNALED(wstatus)) {
			fprintf(stderr, "Command was terminated by %s\n",
				strsignal(WTERMSIG(wstatus)));
			return 0;
		}
	}
	return 1;
}

int ape_cmd_run_sync(ApeCmd cmd)
{
	int p = ape_run_cmd_async(cmd);
	if (p == -1)
		return 0;
	return ape_proc_wait(p);
}

int ape_cmds_run(ApeCmdList cmds)
{
	for (size_t i = 0; i < cmds.count; i++) {
		int p = ape_cmd_run_sync(cmds.items[i]);
		if (!p)
			return 0;
	}
	return 1;
}

/*************************
 * Command Utilities End
 *************************/

/*************************
 * File Utilities Start
 *************************/

int ape_rename(const char *oldname, const char *newname)
{
	if (rename(oldname, newname) == 0) {
		fprintf(stderr, "INFO: %s > %s\n", oldname, newname);
		return 1;
	}
	fprintf(stderr, "ERROR: Couldn't rename file %s: %s\n", oldname,
		strerror(errno));
	return 0;
}

char *ape_objfile_name(char *srcfilename)
{
	ApeStrBuilder sb = { 0 };
	ape_sb_append_str(&sb, strdup(srcfilename));
	ape_sb_append_str(&sb, APE_OBJ_EXTENSION);
	ape_da_append(&sb, 0);
	return sb.items;
}

int ape_needs_rebuild(const char *outfile, char **infiles, size_t len)
{
	struct stat outs;
	if (stat(outfile, &outs) != 0) {
		return 1;
	}
	struct stat ins;
	for (size_t i = 0; i < len; i++) {
		if (stat(infiles[i], &ins) != 0) {
			fprintf(stderr,
				"ERROR: Failed to get stat (of file %s): %s\n",
				infiles[i], strerror(errno));
			return 1;
		}
		if (ins.st_mtime > outs.st_mtime) {
			return 1;
		}
	}
	return 0;
}

int ape_needs_rebuild1(const char *outfile, const char *infile)
{
	return ape_needs_rebuild(outfile, (char **)&infile, 1);
}

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

/*************************
 * File Utilities End
 *************************/

/*************************
 * ApeBuilder Start
 *************************/

ApeCmd ape_gen_build_command(char *srcfilename, uint16_t flags, ApeStrList args)
{
	ApeCmd cmd = { 0 };
	if (!(ape_needs_rebuild1(ape_objfile_name(srcfilename), srcfilename) ||
	      ((flags >> APE_FLAG_REBUILD) && 1)))
		return cmd;
	ape_cmd_append(&cmd, APECC);
	for (size_t i = 0; i < args.count; i++) {
		ape_cmd_append(&cmd, args.items[i]);
	}
	ape_cmd_append(&cmd, APE_BUILD_SRC_ARGS(srcfilename,
						ape_objfile_name(srcfilename)));
	return cmd;
}

ApeCmd ape_gen_link_command(char *outfilename, char **srcfilenames, size_t len,
			    uint16_t flags, ApeStrList args)
{
	ApeCmd cmd = { 0 };
	char **objfilenames = malloc(len);
	for (size_t i = 0; i < len; i++) {
		objfilenames[i] = ape_objfile_name(srcfilenames[i]);
	}
	ape_cmd_append(&cmd, APELD);
	for (size_t i = 0; i < args.count; i++) {
		ape_cmd_append(&cmd, args.items[i]);
	}
	ApeStrBuilder sb = { 0 };
	ape_sb_append_str(&sb, APE_OUTPUT_DIR);
	if ((flags >> APE_FLAG_SHARED_LIB) & 1) {
#ifndef APE_LIB_PREFIX
#pragma GCC warning \
	"You should define APE_LIB_PREFIX if you are building libraries"
#else
		ape_sb_append_str(&sb, APE_LIB_PREFIX);
#endif
		ape_sb_append_str(&sb, outfilename);
#ifndef APE_LIB_SUFFIX
#pragma GCC warning \
	"You should define APE_LIB_SUFFIX if you are building libraries"
#else
		ape_sb_append_str(&sb, APE_LIB_SUFFIX);
#endif
		ape_da_append(&sb, 0);
		ape_cmd_append(&cmd, APE_LINK_ARGS_SHARED_LIB(sb.items));
	} else {
		ape_sb_append_str(&sb, outfilename);
		ape_da_append(&sb, 0);
		ape_cmd_append(&cmd, APE_LINK_ARGS(sb.items));
	}
	if (!(ape_needs_rebuild(sb.items, objfilenames, len) ||
	      ((flags >> APE_FLAG_REBUILD) & 1)))
		return (ApeCmd){ 0 };
	for (size_t i = 0; i < len; i++) {
		ape_cmd_append(&cmd, ape_objfile_name(srcfilenames[i]));
	}
	return cmd;
}

struct {
	size_t capacity;
	size_t count;
	ApeBuilder *items;
} ape__builder_list;

ApeCmdList ape_builder_gen_commands(ApeBuilder *builder)
{
	ApeCmdList cl = { 0 };
	for (size_t i = 0; i < builder->infiles.count; i++) {
		ApeCmd c = ape_gen_build_command(builder->infiles.items[i],
						 builder->flags,
						 builder->extra_build_args);
		if (c.items)
			ape_da_append(&cl, c);
	}
	ApeCmd c = ape_gen_link_command(builder->outfile,
					builder->infiles.items,
					builder->infiles.count, builder->flags,
					builder->extra_link_args);
	if (c.items)
		ape_da_append(&cl, c);
	return cl;
}

void ape_builder_append_file(ApeBuilder *builder, char *path)
{
	ape_da_append(&builder->infiles, path);
}

int ape_builder_append_dir(ApeBuilder *builder, char *path)
{
	DIR *dir = opendir(path);
	struct dirent *entry;
	if (!dir) {
		fprintf(stderr, "ERROR: Could not open directory %s\n", path);
		return 1;
	}
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		if (!ape_endswith(entry->d_name, APE_SRC_EXTENSION))
			continue;
		ApeStrBuilder pathbuilder = { 0 };
		ape_sb_append_str(&pathbuilder, path);
		if (pathbuilder.items[pathbuilder.count - 1] != '/')
			ape_da_append(&pathbuilder, '/');
		ape_sb_append_str(&pathbuilder, entry->d_name);
		ape_da_append(&pathbuilder, 0);
		struct stat statbuf;
		if (stat(pathbuilder.items, &statbuf) < 0) {
			fprintf(stderr, "ERROR: Could not get stat of %s: %s\n",
				pathbuilder.items, strerror(errno));
			return 1;
		}
		if ((statbuf.st_mode & S_IFMT) == S_IFREG)
			ape_builder_append_file(builder,
						strdup(pathbuilder.items));
		ape_da_free(pathbuilder);
	}
	return 0;
}

int ape_builder_append_dir_recursive(ApeBuilder *builder, char *path)
{
	DIR *dir = opendir(path);
	struct dirent *entry;
	if (!dir) {
		fprintf(stderr, "ERROR: Could not open directory %s\n", path);
		return 1;
	}
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		ApeStrBuilder pathbuilder = { 0 };
		ape_sb_append_str(&pathbuilder, path);
		if (pathbuilder.items[pathbuilder.count - 1] != '/')
			ape_da_append(&pathbuilder, '/');
		ape_sb_append_str(&pathbuilder, entry->d_name);
		ape_da_append(&pathbuilder, 0);
		struct stat statbuf;
		if (stat(pathbuilder.items, &statbuf) < 0) {
			fprintf(stderr, "ERROR: Could not get stat of %s: %s\n",
				pathbuilder.items, strerror(errno));
			return 1;
		}
		if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
			ape_builder_append_dir_recursive(
				builder, strdup(pathbuilder.items));
		if (!ape_endswith(entry->d_name, APE_SRC_EXTENSION))
			continue;
		if ((statbuf.st_mode & S_IFMT) == S_IFREG)
			ape_builder_append_file(builder,
						strdup(pathbuilder.items));
		ape_da_free(pathbuilder);
	}
	return 0;
}

int ape_run_builder(ApeBuilder *builder)
{
	ApeCmdList cmds = ape_builder_gen_commands(builder);
	if (!cmds.items)
		fprintf(stderr, "INFO: Nothing to build!\n");
	int r = ape_cmds_run(cmds);
	ape_da_free(cmds);
	return r;
}

/*************************
 * ApeBuilder End
 *************************/

/*************************
 * Execution Start
 *************************/

int ape_run(int argc, char **argv)
{
	for (size_t i = 0; i < ape__builder_list.count; i++) {
		fprintf(stderr, "INFO: Building %s...\n",
			ape__builder_list.items[i].outfile);
		int p = ape_run_builder(&ape__builder_list.items[i]);
		if (!p)
			return 1;
	}
	return 0;
}

/*************************
 * Execution End
 *************************/

#endif
