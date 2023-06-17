#include "VM/Vars/Base.hpp"

#include <utility>
#include "Memory.hpp"

namespace june {
namespace vars {

Base::Base(TypeId type, const u64 &srcId, const u64 &idx,
           const bool callable, const bool attrBased)
    : _type(std::move(type)), _srcId(srcId), _idx(idx), _rc(0) {
  if (callable)
    _info.set(Info::Callable);
  if (attrBased)
    _info.set(Info::AttrBased);
}

auto Base::typeFnId() const -> u64 { return _type.id; }

auto Base::attrExists(const std::string &attr) const -> bool { return false; }
auto Base::attrGet(const std::string &attr) -> Base * { return nullptr; }
auto Base::attrSet(const std::string &attr, Base *val, const bool iref)
    -> void {}

auto Base::operator new(u64 size) -> void * {
  return june::mem::alloc(size);
}

auto Base::operator delete(void *ptr, u64 sz) -> void {
  june::mem::free(ptr, sz);
}

} // namespace vars
} // namespace june
