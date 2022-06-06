#include "VM/Vars/Base.hpp"
#include "VM/Memory.hpp"
#include "VM/State.hpp"

namespace june {

VarBase::VarBase(const std::uintptr_t &type, const size_t &srcId,
                 const size_t &idx, const bool &callable, const bool &attrBased)
    : _type(type), _srcId(srcId), _idx(idx), _refCount(1), _info('\0') {
  if (callable)
    _info |= ViCallable;
  if (attrBased)
    _info |= ViAttrBased;
}
VarBase::~VarBase() {}

std::uintptr_t VarBase::typeFnId() const { return _type; }

bool VarBase::toStr(State &vm, std::string &data, const size_t &srcId,
                    const size_t &idx) {
  if (this->isa<VarString>()) {
    data = this->as<VarString>()->get();
    return true;
  }
  
  VarBase *strFn = nullptr;
  if (this->isAttrBased())
    strFn = this->attrGet("toStr");
  else
    strFn = vm.getTypeFn(this, "toStr");

  if (!strFn) {
    vm.fail(this->srcId(), this->idx(),
            "Unable to convert %s to type `str`: no `toStr` method/attribute",
            vm.getTypeName(this->type()).c_str());
    return false;
  }

  if (!strFn->isa<VarFunc>() && isAttrBased()) {
    // try to convert the attribute to a string
    if (!strFn->toStr(vm, data, srcId, idx)) {
      return false;
    }
  }

  if (!strFn->call(vm, {this}, srcId, idx)) {
    vm.fail(this->srcId(), this->idx(),
            "Unable to convert %s to type `str`: call to `toStr` failed",
            vm.getTypeName(this->type()).c_str());
    return false;
  }

  VarBase *str = vm.stack->pop(false);
  if (!str->isa<VarString>()) {
    vm.fail(this->srcId(), this->idx(),
            "Unable to convert %s to type `str`: `toStr` returned non-string "
            "(found %s)",
            vm.getTypeName(this->type()).c_str(),
            vm.getTypeName(str->type()).c_str());
    varDref(str);
    return false;
  }

  data = str->as<VarString>()->get();
  varDref(str);
  return true;
}

bool VarBase::toBool(State &vm, bool &data, const size_t &srcId,
                     const size_t &idx) {
  if (this->isa<VarBool>()) {
    data = this->as<VarBool>()->get();
    return true;
  }

  VarBase *boolFn = nullptr;
  if (this->isAttrBased())
    boolFn = this->attrGet("toBool");
  else
    boolFn = vm.getTypeFn(this, "toBool");

  if (!boolFn) {
    vm.fail(this->srcId(), this->idx(),
            "Unable to convert %s to type `bool`: no `toBool` method/attribute",
            vm.getTypeName(this->type()).c_str());
    return false;
  }

  if (!boolFn->isa<VarFunc>() && isAttrBased()) {
    // try to convert the attribute to a string
    if (!boolFn->toBool(vm, data, srcId, idx)) {
      return false;
    }
  }

  if (!boolFn->call(vm, {this}, srcId, idx)) {
    vm.fail(this->srcId(), this->idx(),
            "Unable to convert %s to type `bool`: call to `toBool` failed",
            vm.getTypeName(this->type()).c_str());
    return false;
  }

  VarBase *boolVal = vm.stack->pop(false);
  if (!boolVal->isa<VarBool>()) {
    vm.fail(this->srcId(), this->idx(),
            "Unable to convert %s to type `bool`: `toBool` returned non-bool "
            "(found %s)",
            vm.getTypeName(this->type()).c_str(),
            vm.getTypeName(boolVal->type()).c_str());
    varDref(boolVal);
    return false;
  }

  data = boolVal->as<VarBool>()->get();
  varDref(boolVal);
  return true;
}

VarBase *VarBase::call(State &vm, const std::vector<VarBase *> &args,
                       const size_t &srcId, const size_t &idx) {
  VarBase *applyFn = vm.getTypeFn(this, "apply");
  if (!applyFn) {
    vm.fail(this->srcId(), this->idx(), "%s is not a callable object",
            vm.getTypeName(this->type()).c_str());
    return nullptr;
  }

  if (!applyFn->call(vm, args, srcId, idx)) {
    vm.fail(this->srcId(), this->idx(),
            "Unable to call %s: call to `apply` failed",
            vm.getTypeName(this->type()).c_str());
    return nullptr;
  }

  return vm.stack->pop(false);
}

bool VarBase::attrExists(const std::string &attr) const { return false; }
VarBase *VarBase::attrGet(const std::string &attr) { return nullptr; }
void VarBase::attrSet(const std::string &attr, VarBase *val, const bool iref) {}

void *VarBase::operator new(size_t size) {
  return mem::alloc(size);
}

void VarBase::operator delete(void *ptr, size_t sz) {
  mem::free(ptr, sz);
}

void initTypenames(State &vm) {
  vm.registerType<VarAll>("All");
  vm.registerType<VarBool>("bool");
  vm.registerType<VarFloat>("float");
  vm.registerType<VarFunc>("Func");
  vm.registerType<VarInt>("int");
  vm.registerType<VarNil>("nil");
  vm.registerType<VarSrc>("Src");
  vm.registerType<VarString>("string");
  vm.registerType<VarVec>("Vec");
}

} // namespace june
