#ifndef vm_srcfile_hpp
#define vm_srcfile_hpp

#include <cassert>
#include <string>
#include <vector>

#include "../Common.hpp"
#include "OpCodes.hpp"

namespace june {

class SrcFile {
  size_t _id;
  std::string _dir;
  std::string _path;
  std::string _data;
  std::vector<SrcColRange> _cols;

  Bytecode _bytecode;

  bool _isMain;
  bool _isBytecode;

public:
  SrcFile(const std::string &dir, const std::string &path,
          const bool isMain = false);

  err::Errors loadFile();

  void addData(const std::string &data);
  void addCols(const std::vector<SrcColRange> &cols);
  void addBytecode(const std::vector<june::Op> &bytecode);

  inline size_t id() const { return _id; }
  inline const std::string &dir() const { return _dir; }
  inline const std::string &path() const { return _path; }
  inline const std::string &data() const { return _data; }

  Bytecode &bytecode() { return _bytecode; }
  inline bool isMain() const { return _isMain; }
  inline bool isBytecode() const { return _isBytecode; }

  void fail(const size_t &idx, const char *msg, ...) const;
  void fail(const size_t &idx, const char *msg, va_list args) const;
};
} // namespace june

#endif
