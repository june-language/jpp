#include "VM/Stack.hpp"

namespace june {

Stack::Stack() {}
Stack::~Stack() {
  for (auto &val : _vec) {
    varDref(val);
  }
}

void Stack::push(VarBase *val, const bool iref) {
  if (iref)
    varIref(val);
  _vec.push_back(val);
}

VarBase *Stack::pop(const bool dref) {
  if (_vec.size() == 0)
    return nullptr;
  VarBase *back = nullptr;
  back = _vec.back();
  _vec.pop_back();
  if (dref)
    varDref(back);
  return back;
}
} // namespace june
