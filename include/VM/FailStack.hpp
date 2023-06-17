#ifndef vm_failstack_hpp
#define vm_failstack_hpp

#include <deque>
#include <vector>

#include "Vars/Base.hpp"

namespace june::vm {

using namespace june::vars;

class FailStack {
  std::vector<std::deque<Base *>> _stack;

public:
  FailStack();
  ~FailStack();

  inline auto blkAdd() -> void { _stack.push_back(std::deque<Base *>()); }
  inline auto blkRem() -> void {
    for (auto &i : _stack.back())
      varDref(i);
    _stack.pop_back();
  }

  auto push(Base *val, const bool iref = true) -> void;
  auto pop(const bool dref = true) -> Base *;

  inline auto size() -> u64 const { return _stack.size(); }
  inline auto empty() -> bool const { return _stack.empty(); }
  inline auto backEmpty() -> bool const { return _stack.back().empty(); }
};

} // namespace june::vm

#endif
