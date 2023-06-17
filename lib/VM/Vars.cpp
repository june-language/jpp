#include "VM/Vars.hpp"
#include <cassert>

namespace june::vars {

Vars::Vars() : _fnStack(-1) { _fnVars[0] = new Stack(); }

Vars::~Vars() {
  assert(_fnStack == 0 || _fnStack == -1);
  delete _fnVars[0];
}

auto Vars::exists(const std::string &name) -> bool {
  return _fnVars[_fnStack]->exists(name);
}

auto Vars::existsGlobal(const std::string &name) -> bool {
  for (u32 i = _fnStack; i >= 0; i--) {
    if (_fnVars[i]->existsGlobal(name))
      return true;
  }
  return false;
}

auto Vars::get(const std::string &name) -> Base * {
  assert(_fnStack != -1);
  auto *res = _fnVars[_fnStack]->get(name);
  if (res == nullptr && _fnStack != 0) {
    res = _fnVars[0]->get(name);
  }
  return res;
}

auto Vars::blkAdd(const u64 &count) -> void {
  _fnVars[_fnStack]->incTop(count);
  for (auto &s : _stash) {
    _fnVars[_fnStack]->add(s.first, s.second, false);
  }
  _stash.clear();
}

auto Vars::blkRemove(const u64 &count) -> void {
  _fnVars[_fnStack]->decTop(count);
}

auto Vars::pushFn() -> void {
  ++_fnStack;
  if (_fnStack == 0)
    return;
  _fnVars[_fnStack] = new Stack();
}

auto Vars::popFn() -> void {
  if (_fnStack == 0)
    return;
  delete _fnVars[_fnStack];
  _fnVars.erase(_fnStack);
  --_fnStack;
}

auto Vars::stash(const std::string &name, Base *val, const bool iref) -> void {
  if (iref)
    varIref(val);
  _stash[name] = val;
}

auto Vars::unstash() -> void {
  for (auto &s : _stash)
    varDref(s.second);
  _stash.clear();
}

auto Vars::add(const std::string &name, Base *val, const bool iref) -> void {
  _fnVars[_fnStack]->add(name, val, iref);
}

auto Vars::addModule(const std::string &name, Base *val, const bool iref)
    -> void {
  _fnVars[0]->add(name, val, iref);
}

auto Vars::remove(const std::string &name, const bool dref) -> void {
  _fnVars[_fnStack]->remove(name, dref);
}

} // namespace june::vars
