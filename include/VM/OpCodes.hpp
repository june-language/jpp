#ifndef vm_opcodes_hpp
#define vm_opcodes_hpp

#include "Common.hpp"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace june {

enum OpCodes {
  OpCreate, // Create a new variable

  OpStore, // Store a value into a name

  OpLoad,   // Load from operand onto the stack
  OpUnload, // Pop from the stack

  OpJump,      // unconditionally jump to `n`
  OpJumpTrue,  // jump to `n` if top element on stack is true - will not pop if
               // true
  OpJumpFalse, // jump to `n` if top element on stack is false - will not pop if
               // false
  OpJumpTruePop,  // jump to `n` if top element on stack is true - will always
                  // pop
  OpJumpFalsePop, // jump to `n` if top element on stack is false - will always
                  // pop
  OpJumpNil, // jump to `n` if top element on stack is nil (won't pop otherwise)

  OpBodyMarker, // jump to `n` where a body (of a function) ends + 1
  OpMakeFunc,   // create a function object

  OpBlkA, // add count scopes
  OpBlkR, // rem count scopes

  OpCall,       // call a function
  OpMemberCall, // call a member function
  OpAttr,       // get attribute from an object

  OpReturn, // return from a function

  OpPushLoop, // marks a loop beginning for var stack
  OpPopLoop,  // marks a loop ending for var stack

  OpContinue, // continue a loop
  OpBreak,    // break a loop

  OpPushJump, // marks the position to jump to if `or` exists in an expression
  OpPushJumpNamed, // sets the variable name for last jump instruction (after
                   // OpPushJump)
  OpPopJump, // unmarks the position to jump to if `or` exists in an expression

  _OpLast
};

extern const char *OpCodeStrs[_OpLast];

enum OpDataType {
  OdtInt,
  OdtFloat,
  OdtString,
  OdtIdent,
  OdtSize,
  OdtBool,
  OdtNil,

  _OdtLast
};

extern const char *OpDataTypeStrs[_OdtLast];

union OpData {
  size_t sz;
  char *s;
  bool b;
};

struct Op {
  size_t srcId;
  size_t idx;
  OpCodes op;
  OpDataType type;
  OpData data;
};

std::string opAsString(Op op);

struct Bytecode {
private:
  std::vector<Op> bytecode;

public:
  ~Bytecode();

  void add(const size_t &idx, const OpCodes op);
  void adds(const size_t &idx, const OpCodes op, const OpDataType dtype,
            const std::string &data);
  void addb(const size_t &idx, const OpCodes op, const bool &data);
  void addsz(const size_t &idx, const OpCodes op, const size_t &data);
  void addi(const size_t &idx, const OpCodes op, const std::string &data);
  void addf(const size_t &idx, const OpCodes op, const std::string &data);

  OpCodes at(const size_t &pos) const;
  void updatesz(const size_t &pos, const size_t &value);

  inline const std::vector<Op> &get() const { return bytecode; }
  inline std::vector<Op> &getMut() { return bytecode; }
  inline size_t size() const { return bytecode.size(); }
};

struct FileCompatibleOp {
  size_t srcId;
  size_t idx;
  OpCodes op;
  OpDataType type;
  size_t dataIndex;
};

struct FileCompatibleBytecode {
  std::vector<std::pair<OpData, OpDataType>> compressedData;
  std::vector<FileCompatibleOp> bytecode;
};

using DecompressResult = err::Result<std::vector<Op>, std::string>;

FileCompatibleBytecode compressBytecode(const std::vector<Op> &bytecode);
DecompressResult decompressBytecode(const FileCompatibleBytecode &bytecode);

struct SrcColRange {
  size_t begin;
  size_t end;
};

namespace fs {

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef long long i64;
typedef double f64;

struct ValidRead {
  std::vector<Op> bytecode;
  std::vector<SrcColRange> srcRanges;
};

using ReadResult = err::Result<ValidRead, std::string>;

u8 *writeBytecode(const std::vector<Op> &bytecode,
                  const std::vector<SrcColRange> &srcRanges);
ReadResult readBytecode(const u8 *bytecode);

} // namespace fs

} // namespace june

#endif
