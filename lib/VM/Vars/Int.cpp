#include "VM/Vars/Int.hpp"

namespace june {
namespace vars {

Int::Int(const i64 &val, const u64 &srcId, const u64 &idx)
    : Base(TypeId::Of<Int>("Int"), srcId, idx, false, false), _val(val) {}

auto Int::clone(const u64 &srcId, const u64 &idx) -> Base * {
  return new Int(_val, srcId, idx);
}

auto Int::set(Base *from) -> void {
  if (from->isa<Int>()) {
    _val = from->as<Int>()->_val;
  } else {
    _val = 0;
  }
}

auto Int::get() -> i64 & { return _val; }

} // namespace vars
} // namespace june
