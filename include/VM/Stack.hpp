#ifndef vm_stack_hpp
#define vm_stack_hpp

#include <vector>

#include "Vars/Base.hpp"

namespace june {
namespace vm {

using namespace vars;

class Stack {
  std::vector<Base *> _vec;

public:
  Stack();
  ~Stack();

  auto push(Base *val, const bool iref = true) -> void;
  auto pop(const bool dref = true) -> Base *;

  inline auto back() -> Base *& { return _vec.back(); }
  inline auto get() -> std::vector<Base *> & { return _vec; }
  inline auto size() const -> u64 { return _vec.size(); }
  inline auto empty() const -> bool { return _vec.empty(); }
};

} // namespace vm
} // namespace june

#endif
