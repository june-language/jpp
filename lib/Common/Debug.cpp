#include "Common.hpp"
#include <cxxabi.h>

namespace june {
namespace dbg {

auto demangle(const i8 *name) -> std::string {
  i32 status = -4;
  auto *res = abi::__cxa_demangle(name, nullptr, nullptr, &status);
  if (status == 0) {
    std::string ret_val(res);
    free(res);
    return ret_val;
  }
  return "DemangleFail(" + std::string(name) + ")";
}

} // namespace dbg
} // namespace june
