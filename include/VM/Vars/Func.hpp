#ifndef vm_vars_func_hpp
#define vm_vars_func_hpp

#include "Base.hpp"
#include <string>
#include <utility>
#include <vector>

namespace june {
namespace vars {

struct FnBodySpan {
  u64 start;
  u64 end;

  FnBodySpan(const u64 &start, const u64 &end) : start(start), end(end) {}
};

struct FnAssnArg {
  u64 srcId;
  u64 idx;
  std::string name;
  Base *val;

  FnAssnArg(const u64 &srcId, const u64 &idx, std::string name,
            Base *val)
      : srcId(srcId), idx(idx), name(std::move(name)), val(val) {}
};

struct FnData {
  u64 srcId;
  u64 idx;
  std::vector<Base *> args;
  std::vector<FnAssnArg> assnArgs;
  std::unordered_map<std::string, u64> assnArgsMap;
};

using NativeFnPtr = Base *(*)(State &/*vm*/, const FnData &/*fd*/);

union FnBody {
  NativeFnPtr native;
  FnBodySpan june;
};

class Func : public Base {
  std::string _srcName;
  std::vector<std::string> _args;
  std::unordered_map<std::string, Base *> _assnArgs;
  FnBody _body;
  std::string _varArg;
  bool _isNative;

public:
  Func(std::string srcName, std::string varArg,
       std::vector<std::string> args,
       std::unordered_map<std::string, Base *> assnArgs,
       const FnBody &body, const bool isNative, const u64 &srcId,
       const u64 &idx);

  auto clone(const u64 &srcId, const u64 &idx) -> Base * override;
  auto set(Base *from) -> void override;

  auto isNative() const -> bool { return _isNative; }
  auto isJune() const -> bool { return !_isNative; }

  auto srcName() -> std::string & { return _srcName; }
  auto varArg() -> std::string & { return _varArg; }
  auto args() -> std::vector<std::string> & { return _args; }
  auto body() -> FnBody & { return _body; }

  auto call(State &vm, const std::vector<Base *> &args, const u64 &srcId,
            const u64 &idx) -> Base *;
};

} // namespace vars
} // namespace june

#endif
