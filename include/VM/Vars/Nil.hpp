#ifndef vm_vars_nil_hpp
#define vm_vars_nil_hpp

#include "Base.hpp"

namespace june {
namespace vars {

class Nil : public Base {
public:
  Nil(const u64 &srcId, const u64 &idx);

  auto clone(const u64 &srcId, const u64 &idx) -> Base * override;
  void set(Base *val) override;
};

} // namespace vars
} // namespace june

#endif
