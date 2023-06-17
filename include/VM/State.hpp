#ifndef vm_state_hpp
#define vm_state_hpp

#include <deque>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../Common.hpp"
#include "Dylib.hpp"
#include "FailStack.hpp"
#include "SrcFile.hpp"
#include "Vars.hpp"
#include "Vars/Base.hpp"

#define Stringify(x) #x

namespace june {
namespace vm {

using namespace june::vars;

using SrcStack = std::vector<Src *>;
using AllSrcs = std::unordered_mapp<std::string, Src *>;

#define kExecStackMaxDefault 2000

using LoadError = err::Result<SrcFile, err::Error>;

} // namespace vm
} // namespace june

#endif
