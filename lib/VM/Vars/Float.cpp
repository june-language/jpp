#include "VM/Vars/Base.hpp"

namespace june {

VarFloat::VarFloat(const double &val, const size_t &srcId, const size_t &idx)
    : VarBase(type_id<VarFloat>(), srcId, idx, false, false), _data(val) {}

VarFloat::VarFloat(const char *val, const size_t &srcId, const size_t &idx)
    : VarBase(type_id<VarFloat>(), srcId, idx, false, false), _data(0.0) {
  if (val) {
    char *end = nullptr;
    _data = strtod(val, &end);
    if (end == val) {
      _data = 0.0;
    }
  }
}

VarBase *VarFloat::copy(const size_t &srcId, const size_t &idx) {
  return new VarFloat(_data, srcId, idx);
}

double &VarFloat::get() { return _data; }

void VarFloat::set(VarBase *from) {
  if (from->isa<VarFloat>()) {
    _data = AsFloat(from)->get();
  } else if (from->isa<VarInt>()) {
    _data = AsInt(from)->get();
  } else if (from->isa<VarBool>()) {
    _data = AsBool(from)->get();
  } else {
    _data = 0.0;
  }
}

} // namespace june
