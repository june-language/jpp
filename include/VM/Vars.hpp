#ifndef vm_vars_hpp
#define vm_vars_hpp

#include <string>
#include <unordered_map>
#include <vector>

#include "Vars/Base.hpp"

namespace june {

class VarsFrame {
  std::unordered_map<std::string, VarBase *> _vars;

public:
  VarsFrame();
  ~VarsFrame();

  inline const std::unordered_map<std::string, VarBase *> &vars() const {
    return _vars;
  }

  inline bool exists(const std::string &name) {
    return _vars.find(name) != _vars.end();
  }
  VarBase *get(const std::string &name);

  void add(const std::string &name, VarBase *val, const bool iref);
  void rem(const std::string &name, const bool dref);

  static void *operator new(size_t sz);
  static void operator delete(void *ptr, size_t sz);
};

class VarsStack {
  std::vector<size_t> _loopsFrom;
  std::vector<VarsFrame *> _stack;
  size_t _top;

public:
  VarsStack();
  ~VarsStack();

  // checks if a variable exists in the current scope
  bool exists(const std::string &name);

  // checks if a variable exists in any scope
  bool existsGlobal(const std::string &name);

  VarBase *get(const std::string &name);

  void incTop(const size_t &count);
  void decTop(const size_t &count);

  void pushLoop();
  void popLoop();
  void loopContinue();

  void add(const std::string &name, VarBase *val, const bool iref);
  void rem(const std::string &name, const bool dref);
};

class Vars {
  size_t _fnStack;
  std::unordered_map<std::string, VarBase *> _stash;
  std::unordered_map<size_t, VarsStack *> _fnVars;

public:
  Vars();
  ~Vars();

  // checks if a variable exists in the current scope
  bool exists(const std::string &name);

  // checks if a variable exists in any scope
  bool existsGlobal(const std::string &name);

  VarBase *get(const std::string &name);

  void blkAdd(const size_t &count);
  void blkRem(const size_t &count);

  void pushFn();
  void popFn();

  void stash(const std::string &name, VarBase *val, const bool &iref = true);
  void unstash();

  inline void pushLoop() { _fnVars[_fnStack]->pushLoop(); }
  inline void popLoop() { _fnVars[_fnStack]->popLoop(); }
  inline void loopContinue() { _fnVars[_fnStack]->loopContinue(); }

  void add(const std::string &name, VarBase *val, const bool &iref);
  // add a variable to module level unconditionally
  void addm(const std::string &name, VarBase *val, const bool &iref);
  void rem(const std::string &name, const bool &dref);
};

} // namespace june

#endif
