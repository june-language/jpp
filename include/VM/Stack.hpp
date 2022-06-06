#ifndef vm_stack_hpp
#define vm_stack_hpp

#include <vector>

#include "Vars/Base.hpp"

namespace june {

class Stack {
  std::vector<VarBase *> _vec;

public:
  Stack();
  ~Stack();

  void push(VarBase *val, const bool iref = true);
  VarBase *pop(const bool dref = true);

  inline VarBase *&back() { return _vec.back(); }
  inline std::vector<VarBase *> &get() { return _vec; }
  inline size_t size() const { return _vec.size(); }
  inline bool empty() const { return _vec.empty(); }
};
} // namespace june

#endif
