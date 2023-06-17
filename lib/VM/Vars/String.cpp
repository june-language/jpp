#include "VM/Vars/String.hpp"

#include <utility>

namespace june {
namespace vars {

String::String(std::string val, const u64 &srcId, const u64 &idx)
    : Base(TypeId::Of<String>("String"), srcId, idx, false, false), _val(std::move(val)) {}

auto String::clone(const u64 &srcId, const u64 &idx) -> Base * {
  return new String(_val, srcId, idx);
}

auto String::set(Base *from) -> void {
  if (from->isa<String>()) {
    _val = from->as<String>()->_val;
  } else {
    _val = "";
  }
}

auto String::get() -> std::string & { return _val; }

} // namespace vars
} // namespace june
