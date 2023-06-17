#ifndef vm_dylib_hpp
#define vm_dylib_hpp

#include "../Common.hpp"
#include <string>
#include <unordered_map>

#if defined(JUNE_WINDOWS)
#include <windows.h>

#define dlopen(file, mode) LoadLibrary(file)
#define dlclose(handle) FreeLibrary(handle)
#define dlsym(handle, symbol) GetProcAddress(handle, symbol)
#define dlerror() "unknown error"
#else
#include <dlfcn.h>
#endif

namespace june {
namespace vm {

using namespace june::functional;
using namespace june::err;

class DylibManager {
  std::unordered_map<std::string, void *> handles;

public:
  DylibManager();
  ~DylibManager();

  auto load(const std::string &file) -> Result<void *, std::string>;
  auto unload(const std::string &file) -> void;
  auto get(const std::string &file, const std::string &symbol)
      -> Option<void *>;

  inline auto exists(const std::string &file) -> bool {
    return handles.find(file) != handles.end();
  }
};

} // namespace vm
} // namespace june

#endif
