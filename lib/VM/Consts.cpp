#include "VM/Consts.hpp"
#include "VM/OpCodes.hpp"

namespace june {
namespace constants {
VarBase *get(State &vm, const OpDataType type, const OpData &opData,
             const size_t &srcId, const size_t &idx)
{
  switch(type) {
  case OdtBool:
    return opData.b ? vm.tru : vm.fals;
  case OdtNil:
    return vm.nil;
  case OdtInt:
    return make_all<VarInt>(opData.s, srcId, idx);
  case OdtFloat:
    return make_all<VarFloat>(opData.s, srcId, idx);
  case OdtString:
    return make_all<VarString>(opData.s, srcId, idx);
  default:
    return nullptr;
  }
}
}
}
