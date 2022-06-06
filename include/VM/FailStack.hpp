#ifndef vm_failstack_hpp
#define vm_failstack_hpp

#include <deque>
#include <vector>

#include "Vars/Base.hpp"

namespace june {

class FailStack {
  std::vector<std::deque<VarBase *>> _stack;

public:
  FailStack();
  ~FailStack();

  inline void blka() { _stack.push_back(std::deque<VarBase *>()); }
  inline void blkr() {
    for (auto &e : _stack.back())
      varDref(e);
    _stack.pop_back();
  }

  void push(VarBase *val, const bool iref = true);
  VarBase *pop(const bool dref = true);

  inline size_t size() const { return _stack.size(); }
  inline bool empty() const { return _stack.empty(); }
  inline bool backEmpty() const { return _stack.back().empty(); }
};
} // namespace june

#endif
