#ifndef vm_state_hpp
#define vm_state_hpp

#include <deque>
#include <iostream>
#include <string>
#include <unordered_map>

#include "Common.hpp"
#include "Dylib.hpp"
#include "FailStack.hpp"
#include "SrcFile.hpp"
#include "Stack.hpp"
#include "VM/Vars/Base.hpp"
#include "Vars.hpp"

#define Stringify(x) #x

namespace june {

typedef std::vector<VarSrc *> SrcStack;
typedef std::unordered_map<std::string, VarSrc *> AllSrcs;

#define kExecStackMaxDefault 2000

using LoadError = err::Result<SrcFile, err::Error>;

typedef err::Errors (*ReadCodeFn)(const SrcFile *src,
                                  const std::string &srcDir,
                                  const std::string &srcPath, Bytecode &bc,
                                  const bool isMainSrc, const bool exprOnly,
                                  const size_t &beginIdx, const size_t &endIdx);

typedef SrcFile *(*LoadCodeFn)(const std::string &srcFile,
                               const std::string &srcDir, const bool isMainSrc,
                               err::Errors &err, const size_t &beginIdx,
                               const size_t &endIdx);

typedef bool (*ModInitFn)(State &vm, const size_t srcId, const size_t &idx);
typedef void (*ModDeInitFn)();

template <typename T, typename... Args> T *make(Args... args) {
  T *res = new T(args..., 0, 0);
  res->dref();
  return res;
}

template <typename T, typename... Args> T *make_all(Args... args) {
  T *res = new T(args...);
  res->dref();
  return res;
}

struct State {
  bool exitCalled;
  bool execStackCountExceeded;
  size_t exitCode;
  size_t execStackCount;
  size_t execStackMax;

  FailStack fails;

  SrcStack srcStack;
  AllSrcs allSrcs;
  Stack *stack;

  VarBase *tru;
  VarBase *fals;
  VarBase *nil;

  Dylib *dylib;

  VarBase *srcArgs;

  State(const std::string &selfBin, const std::string &selfBase,
        const std::vector<std::string> &args);
  ~State();

  void pushSrc(SrcFile *src, const size_t &idx);
  void pushSrc(const std::string &srcPath);
  void popSrc();

  bool juneModuleExists(std::string &mod, const std::string &ext,
                        std::string &dir);
  bool nativeModuleExists(std::string &mod, const std::string &ext,
                          std::string &dir);
  bool nativeModuleLoad(const std::string &modStr, const size_t &srcId,
                        const size_t &idx);

  err::Errors juneModuleLoad(std::string &modStr, const size_t &srcId,
                             const size_t &idx);

  inline void setModLoadFn(LoadCodeFn fn) { srcLoadCodeFn = fn; }
  inline void setModReadCodeFn(ReadCodeFn fn) { srcReadCodeFn = fn; }

  inline LoadCodeFn getModLoadFn() { return srcLoadCodeFn; }
  inline ReadCodeFn getModReadCodeFn() { return srcReadCodeFn; }

  inline VarSrc *currentSource() const { return srcStack.back(); }
  inline SrcFile *currentSourceFile() const { return srcStack.back()->src(); }

  void globalAdd(const std::string &name, VarBase *val, const bool iref = true);
  VarBase *globalGet(const std::string &name);

  template <typename... T>
  void registerType(const std::string &name, const size_t &srcId = 0,
                    const size_t &idx = 0) {
    setTypeName(type_id<T...>(), name);
    VarTypeId *typeVar = make_all<VarTypeId>(type_id<T...>(), srcId, idx);
    if (srcStack.empty())
      globalAdd(name, typeVar);
    else
      srcStack.back()->addNativeVar(name, typeVar, true, true);
  }

  void addTypeFn(const std::uintptr_t &type, const std::string &name,
                 VarBase *fn, const bool iref);
  template <typename... T>
  void addNativeTypeFn(const std::string &name, NativeFnPtr fn,
                       const size_t &argsCount, const bool isVarArgs,
                       const size_t &srcId, const size_t &idx) {
    addTypeFn(type_id<T...>(), name,
              new VarFunc(srcStack.back()->src()->path(), isVarArgs ? "." : "",
                        std::vector<std::string>(argsCount, ""), {.native = fn},
                        true, srcId, idx),
              true);
  }
  VarBase *getTypeFn(VarBase *val, const std::string &name);

  void setTypeName(const std::uintptr_t &type, const std::string &name);
  std::string getTypeName(const std::uintptr_t &type);
  std::string getTypeName(const VarBase *val);

  inline const std::string &selfBin() const { return _selfBin; }
  inline const std::string &selfBase() const { return _selfBase; }

  void fail(const size_t &srcId, const size_t &idx, const char *msg, ...);
  // `msg` is nullable
  void fail(const size_t &srcId, const size_t &idx, VarBase *val,
            const char *msg, const bool &iref = true);

  bool loadCoreModules();

private:
  LoadCodeFn srcLoadCodeFn;
  ReadCodeFn srcReadCodeFn;

  std::unordered_map<std::string, VarBase *> _globals;
  std::unordered_map<std::uintptr_t, VarsFrame *> _typeFns;
  std::unordered_map<std::uintptr_t, std::string> _typeNames;
  std::unordered_map<std::string, ModDeInitFn> _modDeInitFns;
  std::string _selfBin;
  std::string _selfBase;
};

const char *nativeModuleExt();
const char *juneModuleExt(const bool compiled = false);

namespace vm {

using ExecResult = err::Result<size_t, err::Error>;

// if end == 0, exec until size of bytecode
ExecResult exec(State &vm, const Bytecode *customBytecode = nullptr,
                const size_t &begin = 0, const size_t &end = 0);

} // namespace vm

} // namespace june

#endif
