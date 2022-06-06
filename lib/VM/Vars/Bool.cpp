#include "VM/Vars/Base.hpp"

namespace june {

VarBool::VarBool(const bool data, const size_t &srcId, const size_t &idx)
    : VarBase(type_id<VarBool>(), srcId, idx, false, false), _data(data) {}

bool &VarBool::get() { return _data; }
VarBase *VarBool::copy(const size_t &srcId, const size_t &idx) {
  return new VarBool(_data, srcId, idx);
}
void VarBool::set(VarBase *from) {
  if (from->isa<VarBool>()) {
    _data = from->as<VarBool>()->get();
  } else {
    _data = false;
  }
}

} // namespace june
