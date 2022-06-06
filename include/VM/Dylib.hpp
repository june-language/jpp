#ifndef vm_dylib_hpp
#define vm_dylib_hpp

#include <string>
#include <unordered_map>

namespace june {

class Dylib {
  std::unordered_map<std::string, void *> handles;

public:
  Dylib();
  ~Dylib();

  void *load(const std::string &file);
  void unload(const std::string &file);
  void *get(const std::string &file, const std::string &sym);

  inline bool exists(const std::string &file) {
    return handles.find(file) != handles.end();
  }
};
} // namespace june

#endif
