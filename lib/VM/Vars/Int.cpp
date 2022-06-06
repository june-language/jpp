#include "VM/Vars/Base.hpp"

namespace june {

VarInt::VarInt(const int &val, const size_t &srcId, const size_t &idx)
    : VarBase(type_id<VarInt>(), srcId, idx, false, false), _data(val) {}

VarInt::VarInt(const long long &val, const size_t &srcId, const size_t &idx)
    : VarBase(type_id<VarInt>(), srcId, idx, false, false), _data(val) {}

VarInt::VarInt(const char *val, const size_t &srcId, const size_t &idx)
    : VarBase(type_id<VarInt>(), srcId, idx, false, false), _data(0) {
  if (val) {
    char *end = nullptr;
    _data = strtoll(val, &end, 10);
    if (end == val) {
      _data = 0;
    }
  }
}

VarBase *VarInt::copy(const size_t &srcId, const size_t &idx) {
  return new VarInt(_data, srcId, idx);
}

long long &VarInt::get() { return _data; }
void VarInt::set(VarBase *from) {
  if (from->isa<VarInt>()) {
    _data = AsInt(from)->get();
  } else if (from->isa<VarFloat>()) {
    _data = AsFloat(from)->get();
  } else if (from->isa<VarBool>()) {
    _data = AsBool(from)->get();
  } else {
    _data = 0;
  }
}

} // namespace june
