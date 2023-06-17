#include "VM/Vars/Vec.hpp"
#include "VM/Vars/Bool.hpp"
#include "VM/Vars/Int.hpp"

namespace june {
namespace vars {

Vec::Vec(std::vector<Base *> &val, const bool &refs, const u64 &srcId,
         const u64 &idx)
    : Base(TypeId::Of<Vec>("Vec"), srcId, idx, false, false), _refs(refs) {}

auto Vec::clone(const u64 &srcId, const u64 &idx) -> Base * {
  std::vector<Base *> newVec;
  if (_refs) {
    for (auto &v : _val) {
      varIref(v);
      newVec.push_back(v);
    }
  } else {
    for (auto &v : _val)
      newVec.push_back(v->clone(srcId, idx));
  }
  return new Vec(newVec, _refs, srcId, idx);
}

auto Vec::set(Base *from) -> void {
  if (from->isa<Vec>()) {
    auto *v = static_cast<Vec *>(from);
    if (_refs) {
      for (auto &v : _val)
        varDref(v);
      _val = v->get();
      for (auto &v : _val)
        varIref(v);
    } else {
      for (auto &v : _val)
        delete v;
      _val = v->get();
    }
  } else {
    _val = {};
    _refs = false;
  }
}

auto Vec::attrSet(const std::string &name, Base *val, const bool iref) -> void {
  if (name == "refs") {
    if (val->isa<Bool>()) {
      auto *b = static_cast<Bool *>(val);
      if (b->get()) {
        if (!_refs) {
          for (auto &v : _val) {
            varIref(v);
          }
          _refs = true;
        }
      } else {
        if (_refs) {
          for (auto &v : _val) {
            varDref(v);
          }
          _refs = false;
        }
      }
    }
  } else {
    // find the index of the element
    auto idx = std::stoi(name);
    if (idx < _val.size()) {
      if (_refs) {
        varDref(_val[idx]);
        _val[idx] = val;
        varIref(_val[idx]);
      } else {
        delete _val[idx];
        _val[idx] = val;
      }
    }
  }
}

auto Vec::attrGet(const std::string &name) -> Base * {
  if (name == "refs") {
    return new Bool(_refs, srcId(), idx());
  } if (name == "size") {
    return new Int(_val.size(), srcId(), idx());
  }

  // find the index of the element
  auto idx = std::stoi(name);
  if (idx < _val.size()) {
    return _val[idx];
  }

  return nullptr;
}

auto Vec::attrExists(const std::string &name) const -> bool {
  if (name == "refs" || name == "size") {
    return true;
  } else {
    // find the index of the element
    auto idx = std::stoi(name);
    if (idx < _val.size()) {
      return true;
    }
  }
  return false;
}

auto Vec::get() -> std::vector<Base *> & { return _val; }

} // namespace vars
} // namespace june
