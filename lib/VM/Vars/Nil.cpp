#include "VM/Vars/Base.hpp"

namespace june {

VarNil::VarNil(const size_t &srcId, const size_t &idx)
    : VarBase(type_id<VarNil>(), srcId, idx, false, false) {}

VarBase *VarNil::copy(const size_t &srcId, const size_t &idx) {
  return new VarNil(srcId, idx);
}

void VarNil::set(VarBase *from) {}

}
