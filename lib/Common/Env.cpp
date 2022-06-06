#include "Common.hpp"
#include <cstdlib>
#include <cstring>

#if __APPLE__
#include <mach-o/dyld.h> // for _NSGetExecutablePath()
#elif __FreeBSD__
#include <sys/sysctl.h> // for sysctl()
#include <sys/types.h>
#else
#include <unistd.h> // for readlink()
#endif

std::string june::env::get(const std::string &key) {
  const char *env = getenv(key.c_str());
  return env == NULL ? "" : env;
}

std::string june::env::getProcPath() {
  using namespace june::fs;
  char path[kMaxPathChars];
  memset(path, 0, kMaxPathChars);
#if __linux__ || __ANDROID__
  (void)readlink("/proc/self/exe", path, kMaxPathChars);
#elif __FreeBSD__
  int mib[4];
  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PATHNAME;
  mib[3] = -1;
  size_t sz = kMaxPathChars;
  sysctl(mib, 4, path, &sz, NULL, 0);
#elif __NetBSD__
  readlink("/proc/curproc/exe", path, kMaxPathChars);
#elif __OpenBSD__ || __bsdi__ || __DragonFly__
  readlink("/proc/curproc/file", path, kMaxPathChars);
#elif __APPLE__
  uint32_t sz = kMaxPathChars;
  _NSGetExecutablePath(path, &sz);
#endif
  return path;
}

// C API

extern "C" const char *get(const char *key) {
  return june::string::duplicateAsCString(june::env::get(key));
}

extern "C" const char *getProcPath() {
  return june::string::duplicateAsCString(june::env::getProcPath());
}
