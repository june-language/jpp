#include "VM/Stack.hpp"

namespace june::vm {

Stack::Stack() = default;

Stack::~Stack() {
  for (auto &i : _vec) {
    varDref(i);
  }
}

auto Stack::push(Base *val, const bool iref) -> void {
  if (iref) {
    varIref(val);
  }

  _vec.push_back(val);
}

auto Stack::pop(const bool dref) -> Base * {
  if (_vec.empty())
    return nullptr;

  auto *val = _vec.back();
  _vec.pop_back();
  if (dref)
    varDref(val);
  return val;
}

} // namespace june::vm
