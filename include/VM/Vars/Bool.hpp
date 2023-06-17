#ifndef vm_vars_bool_hpp
#define vm_vars_bool_hpp

#include "Base.hpp"

namespace june {
namespace vars {

class Bool : public Base {
  bool _val;

public:
  Bool(const bool &val, const u64 &srcId, const u64 &idx);

  auto clone(const u64 &srcId, const u64 &idx) -> Base * override;
  auto set(Base *from) -> void override;

  auto get() -> bool &;
};

} // namespace vars
} // namespace june

#endif
