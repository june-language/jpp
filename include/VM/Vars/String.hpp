#ifndef vm_vars_string_hpp
#define vm_vars_string_hpp

#include "Base.hpp"

namespace june {
namespace vars {

class String : public Base {
  std::string _val;

public:
  String(std::string val, const u64 &srcId, const u64 &idx);

  auto clone(const u64 &srcId, const u64 &idx) -> Base * override;
  auto set(Base *from) -> void override;

  auto get() -> std::string &;
};

} // namespace vars
} // namespace june

#endif
