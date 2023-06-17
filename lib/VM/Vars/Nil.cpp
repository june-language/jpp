#include "VM/Vars/Nil.hpp"

namespace june {
namespace vars {

Nil::Nil(const u64 &srcId, const u64 &idx)
    : Base(TypeId::Of<Nil>("Nil"), srcId, idx, false, false) {}

auto Nil::clone(const u64 &srcId, const u64 &idx) -> Base * {
  return new Nil(srcId, idx);
}

auto Nil::set(Base *from) -> void {}

} // namespace vars
} // namespace june
