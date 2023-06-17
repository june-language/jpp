#ifndef vm_vars_src_hpp
#define vm_vars_src_hpp

#include "../SrcFile.hpp"
#include "../Vars.hpp"
#include "Base.hpp"
#include "Func.hpp"

namespace june {
namespace vars {

using june::vm::SrcFile;

// Forward declare `Vars` to avoid circular dependency
class Vars;

class Src : public Base {
  SrcFile *_srcFile;
  Vars *_vars;
  bool _owner;

public:
  Src(SrcFile *srcFile, Vars *vars, const u64 &srcId, const u64 &idx,
      const bool owner = true);
  ~Src();

  auto clone(const u64 &srcId, const u64 &idx) -> Base * override;
  auto set(Base *from) -> void override;

  auto attrExists(const std::string &name) const -> bool override;
  auto attrGet(const std::string &name) -> Base * override;
  auto attrSet(const std::string &name, Base *val, const bool iref)
      -> void override;

  auto addNativeFn(const std::string &name, const NativeFnPtr &fn,
                   const u64 &argsCount = 0, const bool &isVarArgs = false)
      -> void;
  auto addNativeVar(const std::string &name, Base *val, const bool iref = true,
                    const bool moduleLevel = false) -> void;

  auto src() -> SrcFile *;
  auto vars() -> Vars *;
};

// auto initTypeNames(State &vm) -> void;

} // namespace vars
} // namespace june

#endif
