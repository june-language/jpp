#ifndef vm_vars_any_hpp
#define vm_vars_any_hpp

#include "Base.hpp"

namespace june {
namespace vars {

using namespace june::functional;

class Any : public Base {
  // private constructor for cloning
  Any(const Any &src);

  // the actual type and value stored in this variable
  Option<TypeId> _typeId;
  Option<Base *> _val;

public:
  Any(Option<TypeId> typeId, Option<Base *> val, const u64 &srcId,
      const u64 &idx);

  auto underlyingType() const -> Option<TypeId>;
  auto underlyingValue() const -> Option<Base *>;

  auto clone(const u64 &srcId, const u64 &idx) -> Base * override;
  auto set(Base *val) -> void override;
};

} // namespace vars
} // namespace june

#endif
