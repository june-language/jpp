#ifndef vm_vars_int_hpp
#define vm_vars_int_hpp

#include "Base.hpp"

namespace june {
namespace vars {

class Int : public Base {
  i64 _val;

public:
  Int(const i64 &val, const u64 &srcId, const u64 &idx);

  auto clone(const u64 &srcId, const u64 &idx) -> Base * override;
  auto set(Base *from) -> void override;

  auto get() -> i64 &;
};

} // namespace vars
} // namespace june

#endif
