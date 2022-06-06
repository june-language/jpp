#include "VM/FailStack.hpp"

namespace june {

FailStack::FailStack() {}

FailStack::~FailStack() { assert(_stack.size() == 0); }

void FailStack::push(VarBase *val, const bool iref) {
  if (iref)
    varIref(val);
  _stack.back().push_back(val);
}

VarBase *FailStack::pop(const bool dref) {
  if (_stack.size() == 0 || _stack.back().size() == 0)
    return nullptr;
  VarBase *front = _stack.back().front();
  _stack.back().pop_front();
  if (dref)
    varDref(front);
  return front;
}

} // namespace june
