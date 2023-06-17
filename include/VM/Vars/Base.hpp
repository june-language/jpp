#ifndef vm_vars_base_hpp
#define vm_vars_base_hpp

#include "../../Common.hpp"
#include <cassert>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace june {

// Some basic forward declarations
struct State;

namespace vars {

using namespace june::structures;

/// @brief The info enum class for variables
enum Info {
  Callable = 1 << 0,
  AttrBased = 1 << 1,
  LoadAsRef = 1 << 2,
  CopyOnWrite = 1 << 3, // TODO: Implement
  Unmanaged = 1 << 4    // TODO: Implement
};

struct TypeId {
  std::uintptr_t id;
  std::string name;

  TypeId(const std::uintptr_t &id, std::string name)
      : id(id), name(std::move(name)) {}

  template <typename T> static auto Of(const std::string &name = "") -> TypeId {
    return TypeId(reinterpret_cast<std::uintptr_t>(&typeid(T)), name);
  }

  // Comparison operators
  auto operator==(const TypeId &other) const -> bool { return id == other.id; }

  auto operator!=(const TypeId &other) const -> bool { return id != other.id; }
};

class Base {
  std::mutex mtx; // TODO: remove/replace
  TypeId _type;
  u64 _srcId;
  u64 _idx;
  u64 _rc;

  BitFlags<Info> _info;

  template <typename T> static inline auto _typeId() -> std::uintptr_t {
    return reinterpret_cast<std::uintptr_t>(&typeid(T));
  }

  template <typename T> friend auto typeId() -> u64;

public:
  Base(TypeId type, const u64 &srcId, const u64 &idx,
       const bool callable, const bool attrBased);
  virtual ~Base();

  // isA/as

  template <typename T> inline auto isa() const -> bool {
    return _type.id == _typeId<T>();
  }

  template <typename T> inline auto as() -> T * {
    if (!isa<T>())
      return nullptr;
    return reinterpret_cast<T *>(this);
  }

  template <typename T> inline auto as() const -> const T * {
    if (!isa<T>())
      return nullptr;
    return reinterpret_cast<const T *>(this);
  }

  // Clone/Set functions

  virtual auto clone(const u64 &srcId, const u64 &idx) -> Base * = 0;
  virtual auto set(Base *from) -> void = 0;

  // Conversion functions

  auto toStr(State &vm, std::string &data, const u64 &srcId,
             const u64 &idx) -> bool;
  auto toBool(State &vm, bool &data, const u64 &srcId, const u64 &idx)
      -> bool;

  // Set/Get basic info

  inline auto setSrcId(const u64 &srcId) -> void { _srcId = srcId; }
  inline auto setIdx(const u64 &idx) -> void { _idx = idx; }

  inline auto srcId() const -> u64 { return _srcId; }
  inline auto idx() const -> u64 { return _idx; }

  inline auto type() const -> const TypeId { return _type; }
  virtual auto typeFnId() const -> std::uintptr_t;

  // Reference counting

  inline auto iref() -> void {
    std::lock_guard<std::mutex> lock(mtx);
    ++_rc;
  }

  inline auto deref() -> u64 {
    std::lock_guard<std::mutex> lock(mtx);
    assert(_rc > 0);
    return --_rc;
  }

  inline auto refCount() const -> u64 { return _rc; }

  // Info

  inline auto isCallable() const -> bool { return _info[Info::Callable]; }
  inline auto isAttrBased() const -> bool { return _info[Info::AttrBased]; }

  inline auto loadAsRef() const -> bool { return _info[Info::LoadAsRef]; }
  inline auto setLoadAsRef(const bool &val) -> void {
    _info[Info::LoadAsRef] = val;
  }

  // Attributes

  virtual auto attrExists(const std::string &name) const -> bool;
  virtual auto attrSet(const std::string &name, Base *val, const bool iref)
      -> void;
  virtual auto attrGet(const std::string &name) -> Base *;

  // Call

  virtual auto call(State &vm, const std::vector<Base *> &args, Base *&ret,
                    const u64 &srcId, const u64 &idx) -> bool;

  // new/delete

  static auto operator new(u64 sz) -> void *;
  static auto operator delete(void *ptr, u64 sz) -> void;
};

template <typename T> auto typeId() -> std::uintptr_t {
  return Base::_typeId<T>();
}

template <typename T> inline auto varIref(T *var) -> void {
  if (var == nullptr)
    return;
  var->iref();
}

template <typename T> inline auto varDref(T *&var) -> void {
  if (var == nullptr)
    return;
  if (var->deref() == 0) {
    delete var;
    var = nullptr;
  }
}

// used in Standard.Threads
template <typename T> inline auto varDrefConst(const T *&var) -> void {
  if (var == nullptr)
    return;
  if (var->deref() == 0) {
    delete var;
  }
}

} // namespace vars
} // namespace june

#endif
