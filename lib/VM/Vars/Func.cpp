#include "VM/Vars/Func.hpp"

#include <utility>

namespace june {
namespace vars {

Func::Func(std::string srcName, std::string varArg,
           std::vector<std::string> args,
           std::unordered_map<std::string, Base *> assnArgs,
           const FnBody &body, const bool isNative, const u64 &srcId,
           const u64 &idx)
    : Base(TypeId::Of<Func>("Func"), srcId, idx, true, false),
      _srcName(std::move(srcName)), _args(std::move(args)), _assnArgs(std::move(assnArgs)), _body(body),
      _varArg(std::move(varArg)), _isNative(isNative) {}

} // namespace vars
} // namespace june
