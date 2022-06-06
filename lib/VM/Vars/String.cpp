#include "VM/Vars/Base.hpp"

namespace june {

VarString::VarString(const std::string &val, const size_t &srcId,
                     const size_t &idx)
    : VarBase(type_id<VarString>(), srcId, idx, false, false), _data(val) {}

VarBase *VarString::copy(const size_t &srcId, const size_t &idx) {
  return new VarString(_data, srcId, idx);
}
std::string &VarString::get() { return _data; }

void VarString::set(VarBase *from) {
  if (from->isa<VarString>()) {
    _data = AsString(from)->get();
  } else if (from->isa<VarInt>()) {
    _data = std::to_string(AsInt(from)->get());
  } else if (from->isa<VarBool>()) {
    _data = std::to_string(AsBool(from)->get());
  } else {
    _data = "";
  }
}

} // namespace june
