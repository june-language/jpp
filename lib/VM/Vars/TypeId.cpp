#include "VM/Vars/Base.hpp"

namespace june {

VarTypeId::VarTypeId(const std::uintptr_t &val, const size_t &srcId,
                     const size_t &idx)
    : VarBase(type_id<VarTypeId>(), srcId, idx, false, false), _typeId(val) {}

VarBase *VarTypeId::copy(const size_t &srcId, const size_t &idx) {
  return new VarTypeId(_typeId, srcId, idx);
}

void VarTypeId::set(VarBase *from) {
  if (from->isa<VarTypeId>()) {
    _typeId = AsTypeId(from)->get();
  } else {
    _typeId = 0;
  }
}

std::uintptr_t &VarTypeId::get() { return _typeId; }

} // namespace june
