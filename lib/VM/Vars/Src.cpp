#include "VM/SrcFile.hpp"
#include "VM/State.hpp"
#include "VM/Vars/Base.hpp"

namespace june {

VarSrc::VarSrc(SrcFile *src, Vars *vars, const size_t &srcId, const size_t &idx,
               const bool owner)
    : VarBase(type_id<VarSrc>(), srcId, idx, false, true), _src(src),
      _vars(vars), _owner(owner) {}

VarSrc::~VarSrc() {
  if (_owner) {
    if (_vars)
      delete _vars;
    if (_src)
      delete _src;
  }
}

VarBase *VarSrc::copy(const size_t &srcId, const size_t &idx) {
  return new VarSrc(_src, _vars, srcId, idx, false);
}

void VarSrc::set(VarBase *from) {
  if (from->isa<VarSrc>()) {
    if (_owner)
      delete _vars;
    _vars = AsSrc(from)->_vars;
    _owner = false;
    _src = AsSrc(from)->_src;
  } else {
    _src = nullptr;
    _vars = nullptr;
  }
}

bool VarSrc::attrExists(const std::string &name) const {
  return _vars->exists(name);
}

void VarSrc::attrSet(const std::string &name, VarBase *val, const bool iref) {
  _vars->add(name, val, iref);
}

VarBase *VarSrc::attrGet(const std::string &name) { return _vars->get(name); }

void VarSrc::addNativeFn(const std::string &name, NativeFnPtr fn,
                         const size_t &argsCount, const bool &isVarArgs) {
  _vars->add(name,
             new VarFunc(_src->path(), isVarArgs ? "." : "",
                       std::vector<std::string>(argsCount, ""), {.native = fn},
                       true, _src->id(), 0),
             false);
}

void VarSrc::addNativeVar(const std::string &name, VarBase *val,
                          const bool iref, const bool moduleLevel) {
  if (moduleLevel)
    _vars->addm(name, val, iref);
  else
    _vars->add(name, val, iref);
}

SrcFile *VarSrc::src() { return _src; }
Vars *VarSrc::vars() { return _vars; }

} // namespace june
