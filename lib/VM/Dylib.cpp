#include "VM/Dylib.hpp"

namespace june {
namespace vm {

DylibManager::DylibManager() : handles() {}

DylibManager::~DylibManager() {
  for (auto &handle : handles) {
    unload(handle.first);
  }
}

auto DylibManager::load(const std::string &file)
    -> Result<void *, std::string> {
  auto *handle = dlopen(file.c_str(), RTLD_LAZY);
  if (handle == nullptr) {
    return std::string(dlerror());
  }
  handles[file] = handle;
  return handle;
}

auto DylibManager::unload(const std::string &file) -> void {
  auto handle = handles.find(file);
  if (handle != handles.end()) {
    dlclose(handle->second);
    handles.erase(handle);
  }
}

auto DylibManager::get(const std::string &file, const std::string &symbol)
    -> Option<void *> {
  auto handle = handles.find(file);
  if (handle != handles.end()) {
    return dlsym(handle->second, symbol.c_str());
  }
  return nullptr;
}

} // namespace vm
} // namespace june
