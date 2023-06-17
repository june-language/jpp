#include "VM/FailStack.hpp"

namespace june::vm {

FailStack::FailStack() = default;

FailStack::~FailStack() { assert(_stack.size() == 0); }

auto FailStack::push(Base *val, const bool iref) -> void {
  if (iref)
    varIref(val);
  _stack.back().push_back(val);
}

auto FailStack::pop(const bool dref) -> Base * {
  if (_stack.size() == 0 || _stack.back().size() == 0)
    return nullptr;
  auto *front = _stack.back().front();
  if (dref)
    varDref(front);
  _stack.back().pop_front();
  return front;
}

} // namespace june::vm
