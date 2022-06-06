#ifndef vm_consts_hpp
#define vm_consts_hpp

#include "OpCodes.hpp"
#include "State.hpp"
#include "Vars/Base.hpp"

namespace june {
namespace constants {
VarBase *get(State &vm, const OpDataType type, const OpData &opData,
             const size_t &srcId, const size_t &idx);
}
} // namespace june

#endif
