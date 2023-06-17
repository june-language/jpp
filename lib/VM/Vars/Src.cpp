#include "VM/Vars/Src.hpp"

namespace june {
namespace vars {

Src::Src(SrcFile *srcFile, Vars *vars, const u64 &srcId, const u64 &idx,
         const bool owner)
    : Base(TypeId::Of<Src>("Src"), srcId, idx, false, true), _srcFile(srcFile),
      _vars(vars), _owner(owner) {}

Src::~Src() {
  if (_owner) {
    if (_vars) {
      delete _vars;
    }

    if (_srcFile) {
      delete _srcFile;
    }
  }
}

auto Src::clone(const u64 &srcId, const u64 &idx) -> Base * {
  return new Src(_srcFile, _vars, srcId, idx, false);
}

auto Src::set(Base *from) -> void {
  if (from->isa<Src>()) {
    _srcFile = from->as<Src>()->_srcFile;
    _vars = from->as<Src>()->_vars;
  } else {
    _srcFile = nullptr;
    _vars = nullptr;
  }
}

auto Src::attrExists(const std::string &name) const -> bool {
  return _vars->exists(name);
}

auto Src::attrGet(const std::string &name) -> Base * {
  return _vars->get(name);
}

auto Src::attrSet(const std::string &name, Base *val, const bool iref) -> void {
  _vars->add(name, val, iref);
}

auto Src::addNativeFn(const std::string &name, const NativeFnPtr &fn,
                      const u64 &argsCount, const bool &isVarArgs) -> void {
  _vars->add(name,
             new Func(_srcFile->path(), isVarArgs ? "." : "",
                      std::vector<std::string>(argsCount, ""), {},
                      {.native = fn}, true, _srcFile->id(), 0),
             false);
}

auto Src::addNativeVar(const std::string &name, Base *val, const bool iref,
                       const bool moduleLevel) -> void {
  if (moduleLevel)
    _vars->addModule(name, val, iref);
  else
    _vars->add(name, val, iref);
}

auto Src::src() -> SrcFile * { return _srcFile; }
auto Src::vars() -> Vars * { return _vars; }

} // namespace vars
} // namespace june
