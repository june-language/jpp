#include "Common.hpp"

#ifdef JUNE_WINDOWS
#include <Windows.h>
#elif defined(JUNE_LINUX)
#include <unistd.h>
#elif defined(JUNE_MACOS)
#include <mach-o/dyld.h>
#endif

namespace june {

namespace env {

auto get(const std::string &key) -> std::string {
#ifdef JUNE_WINDOWS
  char *value = nullptr;
  size_t size = 0;
  _dupenv_s(&value, &size, key.c_str());
  std::string result = value;
  free(value);
  return result;
#else
  char *value = getenv(key.c_str());
  return value ? value : "";
#endif
}

auto getExecutablePath() -> std::string{
  using namespace june::fs;
  char path[kMaxPathChars];
  memset(path, 0, kMaxPathChars);
#ifdef JUNE_WINDOWS
  GetModuleFileNameA(nullptr, path, kMaxPathChars);
#elif defined(JUNE_LINUX)
  char path[kMaxPathChars];
  ssize_t len = readlink("/proc/self/exe", path, kMaxPathChars);
  if (len == -1) {
    return "";
  }
  path[len] = '\0';
#elif defined(JUNE_MACOS)
  uint32_t size = kMaxPathChars;
  if (_NSGetExecutablePath(path, &size) != 0) {
    return "";
  }
#endif
  return path;
}

}

}
