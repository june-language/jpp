#include "VM/Vars.hpp"
#include "VM/Memory.hpp"
#include "VM/Vars/Base.hpp"
#include <cassert>

namespace june {

VarsFrame::VarsFrame() {}
VarsFrame::~VarsFrame() {
  for (auto &var : _vars) {
    varDref(var.second);
  }
}

VarBase *VarsFrame::get(const std::string &name) {
  if (_vars.find(name) == _vars.end())
    return nullptr;
  return _vars[name];
}

void VarsFrame::add(const std::string &name, VarBase *val, const bool iref) {
  if (_vars.find(name) != _vars.end()) {
    varDref(_vars[name]);
  }
  if (iref)
    varIref(val);
  _vars[name] = val;
}

void VarsFrame::rem(const std::string &name, const bool dref) {
  if (_vars.find(name) == _vars.end())
    return;
  if (dref)
    varDref(_vars[name]);
  _vars.erase(name);
}

void *VarsFrame::operator new(size_t sz) { return mem::alloc(sz); }
void VarsFrame::operator delete(void *ptr, size_t sz) { mem::free(ptr, sz); }

// VarsStack

VarsStack::VarsStack() : _top(0) { _stack.push_back(new VarsFrame()); }
VarsStack::~VarsStack() {
  for (auto layer = _stack.rbegin(); layer != _stack.rend(); layer++) {
    delete *layer;
  }
}

bool VarsStack::exists(const std::string &name) {
  return _stack.back()->exists(name);
}

bool VarsStack::existsGlobal(const std::string &name) {
  for (auto layer = _stack.rbegin(); layer != _stack.rend(); layer++) {
    if ((*layer)->exists(name))
      return true;
  }
  return false;
}

VarBase *VarsStack::get(const std::string &name) {
  for (auto layer = _stack.rbegin(); layer != _stack.rend(); layer++) {
    if ((*layer)->exists(name))
      return (*layer)->get(name);
  }
  return nullptr;
}

void VarsStack::incTop(const size_t &count) {
  for (size_t i = 0; i < count; i++) {
    _stack.push_back(new VarsFrame());
    _top++;
  }
}

void VarsStack::decTop(const size_t &count) {
  if (_top == 0)
    return;
  for (size_t i = 0; i < count && _top > 0; i++) {
    delete _stack.back();
    _stack.pop_back();
    _top--;
  }
}

void VarsStack::pushLoop() {
  _loopsFrom.push_back(_top + 1);
  incTop(1);
}

void VarsStack::loopContinue() {
  assert(_loopsFrom.size() > 0);
  if (_top > _loopsFrom.back()) {
    decTop(_top - _loopsFrom.back());
  }
}

void VarsStack::popLoop() {
  assert(_loopsFrom.size() > 0);
  if (_top > _loopsFrom.back()) {
    decTop(_top - _loopsFrom.back());
  }
}

void VarsStack::add(const std::string &name, VarBase *val, const bool iref) {
  _stack.back()->add(name, val, iref);
}

void VarsStack::rem(const std::string &name, const bool dref) {
  for (auto layer = _stack.rbegin(); layer != _stack.rend(); layer++) {
    if ((*layer)->exists(name)) {
      (*layer)->rem(name, dref);
      return;
    }
  }
}

// Vars

Vars::Vars() : _fnStack(-1) { _fnVars[0] = new VarsStack(); }
Vars::~Vars() {
  assert(_fnStack == 0 || _fnStack == -1);
  delete _fnVars[0];
}

bool Vars::exists(const std::string &name) {
  return _fnVars[_fnStack]->exists(name);
}

bool Vars::existsGlobal(const std::string &name) {
  for (int i = _fnStack; i >= 0; i--) {
    if (_fnVars[i]->existsGlobal(name))
      return true;
  }
  return false;
}

VarBase *Vars::get(const std::string &name) {
  assert(_fnStack != -1);
  VarBase *res = _fnVars[_fnStack]->get(name);
  if (res == nullptr && _fnStack != 0) {
    res = _fnVars[0]->get(name);
  }
  return res;
}

void Vars::blkAdd(const size_t &count) {
  _fnVars[_fnStack]->incTop(count);
  for (auto &s : _stash) {
    _fnVars[_fnStack]->add(s.first, s.second, false);
  }
  _stash.clear();
}

void Vars::blkRem(const size_t &count) { _fnVars[_fnStack]->decTop(count); }

void Vars::pushFn() {
  ++_fnStack;
  if (_fnStack == 0)
    return;
  _fnVars[_fnStack] = new VarsStack();
}

void Vars::popFn() {
  if (_fnStack == 0)
    return;
  delete _fnVars[_fnStack];
  _fnVars.erase(_fnStack);
  --_fnStack;
}

void Vars::stash(const std::string &name, VarBase *val, const bool &iref) {
  if (iref)
    varIref(val);
  _stash[name] = val;
}

void Vars::unstash() {
  for (auto &s : _stash)
    varDref(s.second);
  _stash.clear();
}

void Vars::add(const std::string &name, VarBase *val, const bool &iref) {
  _fnVars[_fnStack]->add(name, val, iref);
}

void Vars::addm(const std::string &name, VarBase *val, const bool &iref) {
  _fnVars[0]->add(name, val, iref);
}

void Vars::rem(const std::string &name, const bool &dref) {
  _fnVars[_fnStack]->rem(name, dref);
}

} // namespace june
