#include "VM/Vars/Base.hpp"

namespace june {

VarAll::VarAll(const size_t &srcId, const size_t &idx)
  : VarBase(type_id<VarAll>(), srcId, idx, false, false) {}

VarBase *VarAll::copy(const size_t &srcId, const size_t &idx) {
  return new VarAll(srcId, idx);
}

void VarAll::set(VarBase *from) {}

}
