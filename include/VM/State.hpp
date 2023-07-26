#ifndef vm_state_hpp
#define vm_state_hpp

#include <deque>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../Common.hpp"
#include "Dylib.hpp"
#include "FailStack.hpp"
#include "SrcFile.hpp"
#include "Vars.hpp"
#include "Vars/Base.hpp"
#include "Vars/Src.hpp"

#define Stringify(x) #x

namespace june {
namespace vm {

using namespace june::vars;

using SrcStack = std::vector<vars::Src *>;
using AllSrcs = std::unordered_map<std::string, vars::Src *>;

#define kExecStackMaxDefault 2000

using LoadError = err::Result<SrcFile, err::Error>;

using ReadCodeFn = std::function<err::Errors(
    const SrcFile *src, const std::string &srcDir, const std::string &srcPath,
    Bytecode &bc, const bool isMainSrc, const bool exprOnly,
    const size_t &beginIdx, const size_t &endIdx)>;
using LoadCodeFn = std::function<SrcFile *(
    const std::string &srcFile, const std::string &srcDir, const bool isMainSrc,
          err::Errors &err, const size_t &beginIdx, const size_t &endIdx)>;

using ModInitFn = std::function<bool(State &vm, const size_t srcId, const size_t &idx)>;
using ModDeInitFn = std::function<void()>;

template <typename T, typename... Args> T *make(Args... args) {
  T *res = new T(args..., 0, 0);
  res->deref();
  return res;
}

template <typename T, typename... Args> T *make_all(Args... args) {
  T *res = new T(args...);
  res->deref();
  return res;
}

struct State {
  bool exitCalled;
  bool execStackOverflow;
  size_t exitCode;
  size_t execStackCount;
  size_t execStackMax;

  FailStack fails;

  SrcStack srcStack;
  AllSrcs allSrcs;
  Stack *stack;

  vars::Base *tru;
  vars::Base *fls;
  vars::Base *nil;

  DylibManager *dylib;

  vars::Base *srcArgs;

  State(const std::string &selfBin, const std::string &selfBase,
        const std::vector<std::string> &args);
  ~State();

  auto pushSrc(SrcFile *src, const size_t &idx) -> void;
  auto pushSrc(const std::string &srcPath) -> void;
  auto popSrc() -> void;

  auto juneModuleExists(std::string &module, const std::string &ext, std::string &dir) -> bool;
  auto nativeModuleExists(std::string &module, const std::string &ext, std::string &dir) -> bool;
  auto nativeModuleLoad(const std::string &modStr, const size_t &srcId, const size_t &idx) -> bool;

  auto juneModuleLoad(std::string &modStr, const size_t &srcId,
                      const size_t &idx) -> err::Errors;

  inline auto setModLoadFn(LoadCodeFn fn) -> void { srcLoadCodeFn = fn; }
  inline auto setModReadCodeFn(ReadCodeFn fn) -> void { srcReadCodeFn = fn; }

  inline auto getModLoadFn() -> LoadCodeFn { return srcLoadCodeFn; }
  inline auto getModReadCodeFn() -> ReadCodeFn { return srcReadCodeFn; }

  inline auto currentSource() const -> vars::Src * { return srcStack.back(); }
  inline auto currentSourceFile() const -> SrcFile * { return srcStack.back()->src(); }

  auto globalAdd(const std::string &name, vars::Base *val, const bool iref = true) -> void;
  auto globalGet(const std::string &name) const -> vars::Base *;

  template <typename... T>
  auto registerType(const std::string &name, const size_t &srcId = 0,
                    const size_t &idx = 0) -> void {
    setTypeName(type_id<T...>(), name);
    vars::TypeId *typeVar = make_all<vars::TypeId>(type_id<T...>(), srcId, idx);
    if (srcStack.empty())
      globalAdd(name, typeVar);
    else
      currentSource()->addNativeVar(name, typeVar, true, true);
  }

  auto addTypeFn(const std::uintptr_t &type, const std::string &name,
                 vars::Base *fn, const bool iref) -> void;

  template <typename... T>
  auto addNativeTypeFn(const std::string &name, NativeFnPtr fn,
                       const size_t &argsCount, const bool isVarArgs, 
                       const size_t &srcId, const size_t &idx) -> void {
    addTypeFn(type_id<T...>(), name,
              new vars::Func(currentSourceFile()->path(), isVarArgs ? "." : "",
                             std::vector<std::string>(argsCount, ""), {.native = fn}, true,
                             srcId, idx),
              true)
  }
  
  auto getTypeFn(vars::Base *val, const std::string &name) -> vars::Base *;

  auto setTypeName(const std::uintptr_t &type, const std::string &name) -> void;
  auto getTypeName(const std::uintptr_t &type) -> std::string;
  auto getTypeName(const vars::Base *val) -> std::string;

  inline auto selfBin() const -> const std::string & { return _selfBin; }
  inline auto selfBase() const -> const std::string & { return _selfBase; }

  auto fail(const size_t &srcId, const size_t &idx, const char *msg, ...);
  // `msg` is nullable
  auto fail(const size_t &srcId, const size_t &idx, vars::Base *val, const char *msg, const bool &iref = true);

  bool loadCoreModules();

private:
  LoadCodeFn srcLoadCodeFn;
  ReadCodeFn srcReadCodeFn;

  std::unordered_map<std::string, vars::Base *> _globals;
  std::unordered_map<std::uintptr_t, vars::Frame *> _typeFns;
  std::unordered_map<std::uintptr_t, std::string> _typeNames;
  std::unordered_map<std::string, ModDeInitFn> _modDeInitFns;
  std::string _selfBin;
  std::string _selfBase;
};

const char *nativeModuleExt();
const char *juneModuleExt(const bool compiled = false);

using ExecResult = err::Result<size_t, err::Error>;

auto exec(State &vm, const Bytecode *customBytecode = nullptr,
          const size_t &begin = 0, const size_t &end = 0) -> ExecResult;

} // namespace vm
} // namespace june

#endif
