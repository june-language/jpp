#ifndef vm_srcfile_hpp
#define vm_srcfile_hpp

#include <cassert>
#include <fmt/core.h>
#include <string>
#include <vector>

#include "../Common.hpp"
#include "OpCodes.hpp"

namespace june {
namespace vm {

class SrcFile {
  u64 _id;
  std::string _dir;
  std::string _path;
  std::string data;

  Bytecode _bc;

  bool _isMain;

public:
  SrcFile(const std::string &dir, const std::string &path,
          const bool isMain = false);

  auto loadFile() -> err::Errors;
  auto addByetcode(const std::vector<Op> &bc) -> void;
  auto snippetForOpIndex(const u64 &idx) const -> std::string;

  inline auto id() const -> u64 { return _id; }
  inline auto dir() const -> const std::string & { return _dir; }
  inline auto path() const -> const std::string & { return _path; }

  inline auto bytecode() -> Bytecode & { return _bc; }
  inline auto isMain() const -> bool { return _isMain; }

  template <typename... Args>
  auto fail(const u64 &idx, const std::string &msg, Args... args) const
      -> void;
};

} // namespace vm
} // namespace june

#endif
