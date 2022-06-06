#include "VM/Dylib.hpp"
#include "c/Dylib.h"

#include <dlfcn.h>

#include <cstdio>
#include <string>
#include <unordered_map>

june::Dylib::Dylib() {}

june::Dylib::~Dylib() {
  for (auto &e : this->handles) {
    if (e.second != nullptr)
      dlclose(e.second);
  }
}

void *june::Dylib::load(const std::string &file) {
  if (this->handles.find(file) == this->handles.end()) {
    auto tmp = dlopen(file.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (tmp == nullptr) {
      fprintf(stderr, "internal error: dylib failed to open %s: %s\n",
              file.c_str(), dlerror());
      return nullptr;
    }
    this->handles[file] = tmp;
  }

  return this->handles[file];
}

void june::Dylib::unload(const std::string &file) {
  if (this->handles.find(file) == this->handles.end())
    return;
  dlclose(this->handles[file]);
  handles.erase(file);
}

void *june::Dylib::get(const std::string &file, const std::string &sym) {
  if (this->handles.find(file) == this->handles.end())
    return nullptr;
  return dlsym(this->handles[file], sym.c_str());
}

// C API

june::Dylib *juneDylibFromC(DylibManagerHandle dylib) {
  return reinterpret_cast<june::Dylib *>(dylib);
}

DylibManagerHandle JuneDylibManagerNew() { return new june::Dylib(); }

void JuneDylibManagerDelete(DylibManagerHandle dylib) {
  delete juneDylibFromC(dylib);
}

void *JuneDylibManagerLoad(DylibManagerHandle dylib, const char *file) {
  return juneDylibFromC(dylib)->load(file);
}

void JuneDylibManagerUnload(DylibManagerHandle handle, void *dylib) {
  juneDylibFromC(handle)->unload(reinterpret_cast<const char *>(dylib));
}

void *JuneDylibManagerGetSymbol(DylibManagerHandle handle, void *dylib,
                                const char *sym) {
  return juneDylibFromC(handle)->get(reinterpret_cast<const char *>(dylib),
                                     sym);
}

bool JuneDylibManagerHasLoaded(DylibManagerHandle handle, const char *file) {
  return juneDylibFromC(handle)->exists(file);
}
