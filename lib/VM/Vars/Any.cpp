#include "VM/Vars/Any.hpp"

namespace june {
namespace vars {

Any::Any(Option<TypeId> typeId, Option<Base *> val, const u64 &srcId,
         const u64 &idx)
    : Base(TypeId::Of<Any>("Any"), srcId, idx, false, false), _typeId(typeId) {
  _val = val;
  _typeId = typeId;

  // should underlying values determine the srcId and idx?
  if (_val.isSome()) {
    setSrcId(_val.unwrap()->srcId());
    setIdx(_val.unwrap()->idx());
  } else {
    setSrcId(srcId);
    setIdx(idx);
  }
}

auto Any::underlyingType() const -> Option<TypeId> { return _typeId; }

auto Any::underlyingValue() const -> Option<Base *> { return _val; }

auto Any::clone(const u64 &srcId, const u64 &idx) -> Base * {
  return new Any(_typeId, _val, srcId, idx);
}

auto Any::set(Base *val) -> void {
  _val = Option<Base *>(val);
  _typeId = Option<TypeId>(val->type());
}

} // namespace vars
} // namespace june
