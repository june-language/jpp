#ifndef vm_vars_hpp
#define vm_vars_hpp

#include <string>
#include <unordered_map>
#include <vector>

#include "Memory.hpp"
#include "Vars/Base.hpp"

namespace june {
namespace vars {

class Frame {
  std::unordered_map<std::string, Base *> _vars;

public:
  Frame();
  ~Frame();

  inline auto vars() const -> const std::unordered_map<std::string, Base *> & {
    return _vars;
  }

  inline auto exists(const std::string &name) const -> bool {
    return _vars.find(name) != _vars.end();
  }

  auto get(const std::string &name) const -> Base *;

  auto add(const std::string &name, Base *var, const bool iref) -> void;
  auto remove(const std::string &name, const bool dref) -> void;

  static auto operator new(u64 sz) -> void *;
  static void operator delete(void *ptr, u64 sz);
};

class Stack {
  std::vector<u64> _loopsFrom;
  std::vector<Frame *> _frames;
  u64 _top;

public:
  Stack();
  ~Stack();

  auto exists(const std::string &name) -> bool;
  auto existsGlobal(const std::string &name) -> bool;

  auto get(const std::string &name) -> Base *;

  auto incTop(const u64 &count) -> void;
  auto decTop(const u64 &count) -> void;

  auto pushLoop() -> void;
  auto popLoop() -> void;
  auto loopContinue() -> void;

  auto add(const std::string &name, Base *val, const bool iref) -> void;
  auto remove(const std::string &name, const bool dref) -> void;
};

class Vars {
  u64 _fnStack;
  std::unordered_map<std::string, Base *> _stash;
  std::unordered_map<u64, Stack *> _fnVars;

public:
  Vars();
  ~Vars();

  auto exists(const std::string &name) -> bool;
  auto existsGlobal(const std::string &name) -> bool;

  auto get(const std::string &name) -> Base *;

  auto blkAdd(const u64 &count) -> void;
  auto blkRemove(const u64 &count) -> void;

  auto pushFn() -> void;
  auto popFn() -> void;

  auto stash(const std::string &name, Base *val, const bool iref) -> void;
  auto unstash() -> void;

  inline auto pushLoop() -> void { _fnVars[_fnStack]->pushLoop(); }
  inline auto popLoop() -> void { _fnVars[_fnStack]->popLoop(); }
  inline auto loopContinue() -> void { _fnVars[_fnStack]->loopContinue(); }

  auto add(const std::string &name, Base *val, const bool iref) -> void;
  // Add a variable to the module level unconditionaly
  auto addModule(const std::string &name, Base *val, const bool iref) -> void;
  auto remove(const std::string &name, const bool dref) -> void;
};

} // namespace vars
} // namespace june

#endif
