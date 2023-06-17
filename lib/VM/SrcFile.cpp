#include "VM/SrcFile.hpp"
#include "Common.hpp"
#include "VM/OpCodes.hpp"

#include <cstdarg>
#include <cstdio>
#include <iostream>

static auto srcId() -> u64 {
  static u64 sid = 0;
  return sid++;
}

namespace june {
namespace vm {

SrcFile::SrcFile(const std::string &dir, const std::string &path,
                 const bool isMain)
    : _id(srcId()), _dir(dir), _path(path), _isMain(isMain) {}

using namespace err;

Error SrcFile::loadFile() {}

} // namespace vm
} // namespace june
