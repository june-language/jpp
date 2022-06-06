#include "VM/State.hpp"
#include "VM/Vars/Base.hpp"
#include <unordered_set>

namespace june {

VarFunc::VarFunc(const std::string &srcName, const std::string &varArg,
             const std::vector<std::string> &args, const FnBody &body,
             const bool isNative, const size_t &srcId, const size_t &idx)
    : VarBase(type_id<VarFunc>(), srcId, idx, true, false), _srcName(srcName),
      _args(args), _body(body), _isNative(isNative), _varArg(varArg) {}

VarBase *VarFunc::copy(const size_t &srcId, const size_t &idx) {
  // should we be able to even copy this?
  // return nullptr;
  return new VarFunc(_srcName, _varArg, _args, _body, _isNative, srcId,
                   idx);
}

void VarFunc::set(VarBase *from) {
  if (from->isa<VarFunc>()) {
    _srcName = from->as<VarFunc>()->srcName();
    _args = from->as<VarFunc>()->args();
    _body = from->as<VarFunc>()->body();
    _isNative = from->as<VarFunc>()->isNative();
  } else {
    _srcName = "";
    _args.clear();
    _body.native = nullptr;
    _isNative = false;
  }
}

bool VarFunc::isNative() const { return _isNative; }
bool VarFunc::isJune() const { return !_isNative; }

std::string &VarFunc::srcName() { return _srcName; }
std::string &VarFunc::varArg() { return _varArg; }
std::vector<std::string> &VarFunc::args() { return _args; }
FnBody &VarFunc::body() { return _body; }

VarBase *VarFunc::call(State &vm, const std::vector<VarBase *> &args,
                     const size_t &srcId, const size_t &idx) {
  if (args.size() - 1 < _args.size()) {
    vm.fail(this->srcId(), this->idx(),
            "too few arguments to function: found %zu, expected %zu",
            args.size() - 1, _args.size());
    return nullptr;
  } else if (args.size() - 1 > _args.size() && _varArg.empty()) {
    vm.fail(this->srcId(), this->idx(),
            "too many arguments to function: found %zu, expected %zu",
            args.size() - 1, _args.size());
    return nullptr;
  }

  if (_isNative) {
    VarBase *res = _body.native(vm, FnData{srcId, idx, args});
    if (res == nullptr)
      return nullptr;
    if (res->refCount() == 0)
      res->setSrcIdAndIdx(this->srcId(), this->idx());
    vm.stack->push(res);
    return vm.nil;
  }

  vm.pushSrc(_srcName);
  Vars *vars = vm.currentSource()->vars();
  if (args[0] != nullptr) {
    vars->stash("self", args[0]);
  }

  std::unordered_set<std::string> foundArgs;
  size_t i = 1;
  for (auto &a : _args) {
    if (i == args.size())
      break;
    vars->stash(a, args[i++]);
    foundArgs.insert(a);
  }

  if (vm::exec(vm, nullptr, _body.june.begin, _body.june.end).isErr()) {
    vars->unstash();
    vm.popSrc();
    return nullptr;
  }

  vm.popSrc();
  return vm.nil;
}

} // namespace june
