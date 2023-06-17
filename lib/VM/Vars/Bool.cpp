#include "VM/Vars/Bool.hpp"

namespace june {
namespace vars {

Bool::Bool(const bool &val, const u64 &srcId, const u64 &idx)
    : Base(TypeId::Of<Bool>("Bool"), srcId, idx, false, false), _val(val) {}

auto Bool::clone(const u64 &srcId, const u64 &idx) -> Base * {
  return new Bool(_val, srcId, idx);
}

auto Bool::set(Base *from) -> void {
  if (from->isa<Bool>())
    _val = from->as<Bool>()->get();
  else
    _val = false;
}

auto Bool::get() -> bool & { return _val; }

} // namespace vars
} // namespace june
