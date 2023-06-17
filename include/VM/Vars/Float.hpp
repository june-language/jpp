#ifndef vm_vars_float_hpp
#define vm_vars_float_hpp

#include "Base.hpp"

namespace june {
namespace vars {

class Float : public Base {
  double _val;

public:
  Float(const double &val, const u64 &srcId, const u64 &idx);

  auto clone(const u64 &srcId, const u64 &idx) -> Base * override;
  auto set(Base *from) -> void override;

  auto get() -> double &;
};

} // namespace vars
} // namespace june

#endif
