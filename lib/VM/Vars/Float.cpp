#include "VM/Vars/Float.hpp"

namespace june {
namespace vars {

Float::Float(const double &val, const u64 &srcId, const u64 &idx)
    : Base(TypeId::Of<Float>("Float"), srcId, idx, false, false), _val(val) {}

auto Float::clone(const u64 &srcId, const u64 &idx) -> Base * {
  return new Float(_val, srcId, idx);
}

auto Float::set(Base *from) -> void {
  if (from->isa<Float>()) {
    _val = from->as<Float>()->_val;
  } else {
    _val = 0.0;
  }
}

auto Float::get() -> double & { return _val; }

} // namespace vars
} // namespace june
