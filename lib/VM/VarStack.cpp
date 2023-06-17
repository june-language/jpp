#include "VM/Vars.hpp"
#include <cassert>

namespace june::vars {

Stack::Stack() : _top(0) { _frames.push_back(new Frame()); }

Stack::~Stack() {
  for (auto &i : _frames) {
    delete i;
  }
}

auto Stack::exists(const std::string &name) -> bool {
  return _frames.back()->exists(name);
}

auto Stack::existsGlobal(const std::string &name) -> bool {
  for (auto &i : _frames) {
    if (i->exists(name))
      return true;
  }
  return false;
}

auto Stack::get(const std::string &name) -> Base * {
  for (auto i = _frames.rbegin(); i != _frames.rend(); ++i) {
    if ((*i)->exists(name))
      return (*i)->get(name);
  }
  return nullptr;
}

auto Stack::incTop(const u64 &count) -> void {
  for (u64 i = 0; i < count; ++i) {
    _frames.push_back(new Frame());
    _top++;
  }
}

auto Stack::decTop(const u64 &count) -> void {
  if (_top == 0)
    return;
  for (u64 i = 0; i < count && _top > 0; ++i) {
    delete _frames.back();
    _frames.pop_back();
    _top--;
  }
}

auto Stack::pushLoop() -> void {
  _loopsFrom.push_back(_top + 1);
  incTop(1);
}

auto Stack::popLoop() -> void {
  assert(_loopsFrom.size() > 0);
  if (_top > _loopsFrom.back()) {
    decTop(_top - _loopsFrom.back());
  }
}

auto Stack::loopContinue() -> void {
  assert(_loopsFrom.size() > 0);
  if (_top > _loopsFrom.back()) {
    decTop(_top - _loopsFrom.back());
  }
}

auto Stack::add(const std::string &name, Base *val, const bool iref) -> void {
  _frames.back()->add(name, val, iref);
}

auto Stack::remove(const std::string &name, const bool dref) -> void {
  for (auto i = _frames.rbegin(); i != _frames.rend(); ++i) {
    if ((*i)->exists(name)) {
      (*i)->remove(name, dref);
      return;
    }
  }
}

} // namespace june::vars
