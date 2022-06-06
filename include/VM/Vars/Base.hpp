#ifndef vm_vars_base_hpp
#define vm_vars_base_hpp

#include <cassert>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../SrcFile.hpp"

namespace june {

enum VarInfo {
  ViCallable = 1 << 0,
  ViAttrBased = 1 << 1,
  ViLoadAsRef = 1 << 2,
  // ViUnmanaged = 1 << 3
};

struct State;
class VarBase {
  std::mutex mtx; // TODO: remove/replace
  std::uintptr_t _type;
  size_t _srcId;
  size_t _idx;
  size_t _refCount;

  char _info;

  template <typename T> static inline std::uintptr_t _type_id() {
    return reinterpret_cast<std::uintptr_t>(&_type_id<T>);
  }

  template <typename T> friend size_t type_id();

public:
  VarBase(const std::uintptr_t &type, const size_t &srcId, const size_t &idx,
          const bool &callable, const bool &attrBased);
  virtual ~VarBase();

  template <typename T> bool isa() const {
    return _type == VarBase::_type_id<T>();
  }

  template <typename T> T *as() {
    if (!isa<T>())
      return nullptr;
    return reinterpret_cast<T *>(this);
  }

  virtual VarBase *copy(const size_t &srcId, const size_t &idx) = 0;
  virtual void set(VarBase *from) = 0;

  bool toStr(State &vm, std::string &data, const size_t &srcId,
             const size_t &idx);
  bool toBool(State &vm, bool &data, const size_t &srcId, const size_t &idx);

  inline void setSrcIdAndIdx(const size_t &srcId, const size_t &idx) {
    _srcId = srcId;
    _idx = idx;
  }

  inline std::uintptr_t type() const { return _type; }
  virtual std::uintptr_t typeFnId() const;

  inline size_t srcId() const { return _srcId; }
  inline size_t idx() const { return _idx; }

  inline void iref() {
    std::lock_guard<std::mutex> lock(mtx);
    _refCount++;
  }

  inline size_t dref() {
    std::lock_guard<std::mutex> lock(mtx);
    assert(_refCount > 0);
    _refCount--;
    return _refCount;
  }

  inline size_t refCount() const { return _refCount; }

  inline bool isCallable() const { return _info & VarInfo::ViCallable; }
  inline bool isAttrBased() const { return _info & VarInfo::ViAttrBased; }

  inline bool isLoadAsRef() const { return _info & VarInfo::ViLoadAsRef; }
  inline void setLoadAsRef() { _info |= VarInfo::ViLoadAsRef; }
  inline void unsetLoadAsRef() { _info &= ~VarInfo::ViLoadAsRef; }

  virtual VarBase *call(State &vm, const std::vector<VarBase *> &args,
                        const size_t &srcId, const size_t &idx);

  virtual bool attrExists(const std::string &attr) const;
  virtual void attrSet(const std::string &attr, VarBase *val, const bool iref);
  virtual VarBase *attrGet(const std::string &attr);

  static void *operator new(size_t sz);
  static void operator delete(void *ptr, size_t sz);
};

template <typename T> std::uintptr_t type_id() {
  return VarBase::_type_id<T>();
}

template <typename T> inline void varIref(T *var) {
  if (var == nullptr)
    return;
  var->iref();
}

template <typename T> inline void varDref(T *&var) {
  if (var == nullptr)
    return;
  var->dref();
  if (var->refCount() == 0) {
    delete var;
    var = nullptr;
  }
}

// used in June's Standard.Threads module
template <typename T> inline void varDrefConst(const T *var) {
  if (var == nullptr)
    return;
  var->dref();
  if (var->refCount() == 0) {
    delete var;
  }
}

class VarAll : public VarBase {
public:
  VarAll(const size_t &srcId, const size_t &idx);

  VarBase *copy(const size_t &srcId, const size_t &idx);
  void set(VarBase *from);
};

class VarNil : public VarBase {
public:
  VarNil(const size_t &srcId, const size_t &idx);

  VarBase *copy(const size_t &srcId, const size_t &idx);
  void set(VarBase *from);
};
#define AsNil(x) static_cast<VarNil *>(x)

class VarTypeId : public VarBase {
  std::uintptr_t _typeId;

public:
  VarTypeId(const std::uintptr_t &typeId, const size_t &srcId,
            const size_t &idx);

  VarBase *copy(const size_t &srcId, const size_t &idx);
  void set(VarBase *from);

  std::uintptr_t &get();
};
#define AsTypeId(x) static_cast<VarTypeId *>(x)

class VarBool : public VarBase {
  bool _data;

public:
  VarBool(const bool data, const size_t &srcId, const size_t &idx);

  VarBase *copy(const size_t &srcId, const size_t &idx);
  void set(VarBase *from);

  bool &get();
};
#define AsBool(x) static_cast<VarBool *>(x)

class VarInt : public VarBase {
  long long _data;

public:
  VarInt(const int &val, const size_t &srcId, const size_t &idx);
  VarInt(const long long &val, const size_t &srcId, const size_t &idx);
  VarInt(const char *val, const size_t &srcId, const size_t &idx);

  VarBase *copy(const size_t &srcId, const size_t &idx);
  void set(VarBase *from);

  long long &get();
};
#define AsInt(x) static_cast<VarInt *>(x)

class VarFloat : public VarBase {
  double _data;

public:
  VarFloat(const double &val, const size_t &srcId, const size_t &idx);
  VarFloat(const char *val, const size_t &srcId, const size_t &idx);

  VarBase *copy(const size_t &srcId, const size_t &idx);
  void set(VarBase *from);

  double &get();
};
#define AsFloat(x) static_cast<VarFloat *>(x)

class VarString : public VarBase {
  std::string _data;

public:
  VarString(const std::string &val, const size_t &srcId, const size_t &idx);

  VarBase *copy(const size_t &srcId, const size_t &idx);
  void set(VarBase *from);

  std::string &get();
};
#define AsString(x) static_cast<VarString *>(x)

class VarVec : public VarBase {
  std::vector<VarBase *> _data;
  bool _refs;

public:
  VarVec(const std::vector<VarBase *> &val, const bool &refs,
         const size_t &srcId, const size_t &idx);
  ~VarVec();

  VarBase *copy(const size_t &srcId, const size_t &idx);
  void set(VarBase *from);

  void attrSet(const std::string &attr, VarBase *val, const bool iref);
  VarBase *attrGet(const std::string &attr);
  bool attrExists(const std::string &attr) const;

  std::vector<VarBase *> &get();
  bool isRefVec();
};
#define AsVec(x) static_cast<VarVec *>(x)

struct FnBodySpan {
  size_t begin;
  size_t end;
};

// TODO: assn args? ex. fn(x, y, arg = z)
//                               |   --- assn val
//                               --- assn name

// struct FnAssnArg {
//   size_t srcId;
//   size_t idx;
//   std::string name;
//   VarBase *val;
// };

struct FnData {
  size_t srcId;
  size_t idx;
  std::vector<VarBase *> args;
  // std::vector<FnAssnArg> assnArgs;
  // std::unordered_map<std::string, size_t> assnArgsLoc;
};

typedef VarBase *(*NativeFnPtr)(State &vm, const FnData &data);

union FnBody {
  NativeFnPtr native;
  FnBodySpan june;
};

class VarFunc : public VarBase {
  std::string _srcName;
  std::vector<std::string> _args;
  // std::unordered_map<std::string, VarBase *> _assnArgs;
  FnBody _body;
  std::string _varArg;
  bool _isNative;

public:
  VarFunc(const std::string &srcName,
          const std::string &varArg, const std::vector<std::string> &args,
          // const std::unordered_map<std::string, VarBase *> &assnArgs,
          const FnBody &body, const bool isNative, const size_t &srcId,
          const size_t &idx);

  VarBase *copy(const size_t &srcId, const size_t &idx);
  void set(VarBase *from);

  bool isNative() const;
  bool isJune() const;

  std::string &srcName();
  std::string &varArg();
  std::vector<std::string> &args();
  FnBody &body();

  VarBase *call(State &vm, const std::vector<VarBase *> &args,
                const size_t &srcId, const size_t &idx);
};
#define AsFunc(x) static_cast<VarFunc *>(x)

class Vars;
class VarSrc : public VarBase {
  SrcFile *_src;
  Vars *_vars;
  bool _owner;

public:
  VarSrc(SrcFile *src, Vars *vars, const size_t &srcId, const size_t &idx,
         const bool owner = true);
  ~VarSrc();

  VarBase *copy(const size_t &srcId, const size_t &idx);
  void set(VarBase *from);

  bool attrExists(const std::string &name) const;
  void attrSet(const std::string &name, VarBase *val, const bool iref);
  VarBase *attrGet(const std::string &name);

  void addNativeFn(const std::string &name, NativeFnPtr fn,
                   const size_t &argsCount = 0, const bool &isVarArgs = false);
  void addNativeVar(const std::string &name, VarBase *var,
                    const bool iref = true, const bool moduleLevel = false);

  SrcFile *src();
  Vars *vars();
};
#define AsSrc(x) static_cast<VarSrc *>(x)

void initTypenames(State &vm);

} // namespace june

#endif
