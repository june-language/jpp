#include "VM/State.hpp"
#include "VM/Vars/Base.hpp"
#include <vector>

namespace june {

VarVec::VarVec(const std::vector<VarBase *> &val, const bool &refs,
               const size_t &srcId, const size_t &idx)
    : VarBase(type_id<VarVec>(), srcId, idx, refs, false), _data(val),
      _refs(refs) {}

VarVec::~VarVec() {
  for (auto &v : _data)
    varDref(v);
}

VarBase *VarVec::copy(const size_t &srcId, const size_t &idx) {
  std::vector<VarBase *> newVec;
  if (_refs) {
    for (auto &v : _data) {
      varIref(v);
      newVec.push_back(v);
    }
  } else {
    for (auto &v : _data)
      newVec.push_back(v->copy(srcId, idx));
  }
  return new VarVec(newVec, _refs, srcId, idx);
}

std::vector<VarBase *> &VarVec::get() { return _data; }

bool VarVec::isRefVec() { return _refs; }
void VarVec::set(VarBase *from) {
  if (from->isa<VarVec>()) {
    _data.clear();
    for (auto &v : AsVec(from)->get()) {
      varIref(v);
      _data.push_back(v);
    }
    _refs = AsVec(from)->isRefVec();
  } else {
    _data.clear();
  }
}

VarBase *VarVec::attrGet(const std::string &attr) {
  if (attr == "size")
    return make_all<VarInt>((long long)_data.size(), this->srcId(), this->idx());
  return nullptr;
}

void VarVec::attrSet(const std::string &attr, VarBase *val, const bool iref) {
  // currently no attributes
  // todo: implement attributes
}

bool VarVec::attrExists(const std::string &attr) const {
  return attr == "size";
}

}
