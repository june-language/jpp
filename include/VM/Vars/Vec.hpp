#ifndef vm_vars_vec_hpp
#define vm_vars_vec_hpp

#include "Base.hpp"

namespace june {
namespace vars {

class Vec : public Base {
  std::vector<Base *> _val;
  bool _refs;

public:
  Vec(std::vector<Base *> &val, const bool &refs, const u64 &srcId,
      const u64 &idx);

  auto clone(const u64 &srcId, const u64 &idx) -> Base * override;
  auto set(Base *from) -> void override;

  auto attrSet(const std::string &name, Base *val, const bool iref)
      -> void override;
  auto attrGet(const std::string &name) -> Base * override;
  auto attrExists(const std::string &name) const -> bool override;

  auto get() -> std::vector<Base *> &;
  inline auto isRefs() -> bool & { return _refs; }
};

} // namespace vars
} // namespace june

#endif
