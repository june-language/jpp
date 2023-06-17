#include "VM/Vars.hpp"

namespace june {
namespace vars {

Frame::Frame() = default;
Frame::~Frame() {
  for (auto &var : _vars) {
    varDref(var.second);
  }
}

auto Frame::get(const std::string &name) const -> Base * {
  auto it = _vars.find(name);
  if (it == _vars.end()) {
    return nullptr;
  }
  return it->second;
}

auto Frame::add(const std::string &name, Base *var, const bool iref) -> void {
  auto it = _vars.find(name);
  if (it != _vars.end()) {
    varDref(it->second);
  }
  _vars[name] = var;
  if (iref) {
    varIref(var);
  }
}

auto Frame::remove(const std::string &name, const bool dref) -> void {
  auto it = _vars.find(name);
  if (it != _vars.end()) {
    if (dref) {
      varDref(it->second);
    }
    _vars.erase(it);
  }
}

auto Frame::operator new(u64 sz) -> void * { return mem::alloc(sz); }
void Frame::operator delete(void *ptr, u64 sz) { mem::free(ptr, sz); }

} // namespace vars
} // namespace june
