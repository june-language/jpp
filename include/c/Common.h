#ifndef common_env_h
#define common_env_h

#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// june::args

typedef struct Arg *ArgHandle;

void JuneArgsAddArgument(const char *queryName, const char *shortName,
                         const char *longName, const char *description,
                         bool hasValue, bool shouldNewline);
void JuneArgsParseArguments(int argc, char **argv);
bool JuneArgsArgumentExists(const char *queryName);
bool JuneArgsPositionalExists(int position);
ArgHandle JuneArgsGetArgument(const char *queryName);
ArgHandle JuneArgsGetPositional(int position);
void JuneArgsModifyArgument(const char *queryName, const char *newValue);
ArgHandle *JuneArgsGetPositionals();
bool JuneArgsAnyArgumentExists();
void JuneArgsPrintHelp(const char *argv0);

// june::string

bool endsWith(const char *str, const char *suffix);

const char **split(const char *str, const char *delimiter);
static void freeSplit(const char **split) { free((void *)split); }

const char *join(const char **strs, const char *delimiter);

const char *toLower(const char *str);
const char *toUpper(const char *str);

// june::fs

const int kMaxPathChars = 1024;

bool exists(const char *path, bool *didError, const char **error);

const char *absPath(const char *path, const char **parentDir,
                    bool dirAddDoubleDot);

bool isAbsolute(const char *path);
bool isDir(const char *path);

const char *dirName(const char *path);

const char *cwd(bool *didError, const char **error);
const char *home(bool *didError, const char **error);

const char *relativePath(const char *path, const char *baseDir);

typedef bool (*SearchMatcher)(const char *path);
const char **search(const char *path, SearchMatcher matcher);

// june::env

const char *get(const char *key);
const char *getProcPath();

#ifdef __cplusplus
}
#endif

#endif
