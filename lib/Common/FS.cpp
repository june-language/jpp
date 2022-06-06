#include "Common.hpp"
#include "c/Common.h"
#include <sys/stat.h>

#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <filesystem>

#ifndef _WIN32
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#else
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <windows.h>
#endif

using namespace june::err;

Result<std::string, Error> june::fs::readFile(const std::string &path){
  using Res = Result<std::string, Error>;
  std::ifstream file(path);
  if (!file.is_open())
    return Res::Err(Error(ErrFileIo, "failed to open file for reading: " + path));
  std::stringstream buffer;
  buffer << file.rdbuf();
  return Res::Ok(buffer.str());
}

Result<bool, Error> june::fs::exists(const std::string &path) {
#if defined(_WIN32)
  return _access(path.c_str(), 0) == 0;
#else
  if (access(path.c_str(), F_OK) == 0)
    return Result<bool, Error>(true);
  else if (errno == ENOENT)
    return Result<bool, Error>(false);
  else
    return Result<bool, Error>(Error(ErrFileIo, "access"));
#endif
}

std::string june::fs::absPath(const std::string &path, std::string *parentDir,
                              const bool &dirAddDoubleDot) {
  char abs[kMaxPathChars];
  char abs_tmp[kMaxPathChars];
#if defined(_WIN32)
  _fullpath(abs, loc.c_str(), kMaxPathChars);
#else
  realpath(path.c_str(), abs);
#endif
  if (parentDir != nullptr) {
    std::string _abs = abs;
    *parentDir = _abs.substr(0, _abs.find_last_of('/'));
    if (dirAddDoubleDot) {
      *parentDir += "/..";
      realpath(parentDir->c_str(), abs_tmp);
      *parentDir = abs_tmp;
    }
  }
  return abs;
}

bool june::fs::isAbsolute(const std::string &path) {
#if defined(_WIN32)
  return path.length() > 1 && path[1] == ':';
#else
  return path.length() > 0 && path[0] == '/';
#endif
}

bool june::fs::isDir(const std::string &path) {
#if defined(_WIN32)
  return _access(path.c_str(), 0) == 0 && _access(path.c_str(), 4) == 0;
#else
  struct stat s;
  return stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
#endif
}

std::string june::fs::dirName(const std::string &path) {
  std::string parentDir;
  absPath(path, &parentDir);
  return parentDir;
}

Result<std::string, Error> june::fs::cwd() {
  char cwd[kMaxPathChars] = "\0";

#if defined(_WIN32)
  _getcwd(cwd, kMaxPathChars);
#else
  if (getcwd(cwd, kMaxPathChars) == nullptr)
    return Result<std::string, Error>(Error(ErrFileIo, "getcwd failed"));
#endif

  return Result<std::string, Error>(cwd[0] == '\0' ? "" : cwd);
}

Result<std::string, Error> june::fs::home() {
#if defined(_WIN32)
  return Result<std::string, Error>(
      Error(ErrFileIo, "june::fs::home() not implemented on windows"));
#else
  struct passwd *pw = getpwuid(getuid());
  if (pw == nullptr)
    return Result<std::string, Error>(Error(ErrFileIo, "getpwuid failed"));
  return Result<std::string, Error>(pw->pw_dir);
#endif
}

std::vector<std::string>
commonPath(const std::vector<std::vector<std::string>> &paths) {
  if (paths.empty())
    return {};

  std::vector<std::string> s1 = paths[0];
  std::vector<std::string> s2 = paths[1];
  for (size_t i = 0; i < s1.size(); ++i) {
    std::string c = s1[i];
    if (c != s2[i]) {
      std::vector<std::string> res;
      for (size_t j = 0; j < i; ++j)
        res.push_back(s1[j]);
      return res;
    }
  }
  return s1;
}

std::string june::fs::relativePath(const std::string &path,
                                   const std::string &refDir) {
  if (path.empty())
    return "";
  auto cur_dir = cwd();
  if (cur_dir.isErr())
    return "";
  std::string dir = refDir.empty() ? *cur_dir.getOk() : refDir;

  std::string cur = absPath(dir);
  std::string abs = absPath(path);

  auto absParts = june::string::split(abs, "/");
  auto curParts = june::string::split(cur, "/");

  size_t i = commonPath({absParts, curParts}).size();

  std::vector<std::string> relParts;
  for (int j = 0; j < curParts.size() - i; ++j) {
    relParts.push_back("..");
  }
  for (int j = i; j < absParts.size(); ++j) {
    relParts.push_back(absParts[j]);
  }

  std::string res = june::string::join(relParts, "/");
  if (res.find('/') == std::string::npos)
    return "./" + res;
  return res;
}

std::vector<std::string>
june::fs::search(const std::string &dir,
                 const std::function<bool(const std::string &)> &matcher) {
  // recursively search for files (even in subdirectories)
  std::vector<std::string> files;
  std::vector<std::string> dirs;

  auto existence = exists(dir);
  if (existence.isErr() || !*existence.getOk())
    return {};

  dirs.push_back(dir);
  while (!dirs.empty()) {
    std::string cur = dirs.back();
    dirs.pop_back();
    DIR *dp = opendir(cur.c_str());
    if (dp == nullptr)
      continue;
    struct dirent *ep;
    while ((ep = readdir(dp)) != nullptr) {
      std::string name = ep->d_name;
      if (name == "." || name == "..")
        continue;
      std::string path = cur + "/" + name;
      // if it's a directory, add it to the list of dirs to search
      // and continue (don't match directories)
      if (ep->d_type == DT_DIR) {
        dirs.push_back(path);
        continue;
      }
      if (matcher(path))
        files.push_back(path);
    }
    closedir(dp);
  }

  return files;
}

// C API

extern "C" bool exists(const char *path, bool *didError, const char **error) {
  auto res = june::fs::exists(path);
  if (res.isErr()) {
    if (didError != nullptr)
      *didError = true;
    if (error != nullptr)
      *error = june::string::duplicateAsCString(res.getErr()->toString());
    return false;
  }
  return *res.getOk();
}

extern "C" const char *absPath(const char *path, const char **parentDir,
                               bool dirAddDoubleDot) {
  std::string pDir;
  std::string res = june::fs::absPath(path, &pDir, dirAddDoubleDot);
  if (parentDir != nullptr)
    *parentDir = june::string::duplicateAsCString(pDir);
  return june::string::duplicateAsCString(res);
}

extern "C" bool isAbsolute(const char *path) {
  return june::fs::isAbsolute(path);
}

extern "C" bool isDir(const char *path) { return june::fs::isDir(path); }

extern "C" const char *dirName(const char *path) {
  return june::string::duplicateAsCString(june::fs::dirName(path));
}

extern "C" const char *cwd(bool *didError, const char **error) {
  auto res = june::fs::cwd();
  if (res.isErr()) {
    if (didError != nullptr)
      *didError = true;
    if (error != nullptr)
      *error = june::string::duplicateAsCString(res.getErr()->toString());
    return nullptr;
  }
  return june::string::duplicateAsCString(*res.getOk());
}

extern "C" const char *home(bool *didError, const char **error) {
  auto res = june::fs::home();
  if (res.isErr()) {
    if (didError != nullptr)
      *didError = true;
    if (error != nullptr)
      *error = june::string::duplicateAsCString(res.getErr()->toString());
    return nullptr;
  }
  return june::string::duplicateAsCString(*res.getOk());
}

extern "C" const char *relativePath(const char *path, const char *baseDir) {
  return june::string::duplicateAsCString(
      june::fs::relativePath(path, baseDir));
}

extern "C" const char **search(const char *path, SearchMatcher matcher) {
  std::vector<std::string> res =
      june::fs::search(path, [matcher](const std::string &path) {
        return matcher(path.c_str());
      });

  const char **files = new const char *[res.size()];
  for (size_t i = 0; i < res.size(); ++i)
    files[i] = june::string::duplicateAsCString(res[i]);
  return files;
}
