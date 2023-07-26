#ifndef vm_vars_float_hpp
#define vm_vars_float_hpp

#include "Base.hpp"

namespace june {
namespace vars {

class Float : public Base {
  f64 _val;

public:
  Float(const f64 &val, const u64 &srcId, const u64 &idx);

  auto clone(const u64 &srcId, const u64 &idx) -> Base * override;
  auto set(Base *from) -> void override;

  auto get() -> f64 &;
};

} // namespace vars
} // namespace june

#endif
