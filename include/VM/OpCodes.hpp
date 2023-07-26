#ifndef vm_opcodes_hpp
#define vm_opcodes_hpp

#include "../Common.hpp"
#include "../JuneConfig.hpp"
#include <cstdio>
#include <cstdlib>
#include <fmt/core.h>
#include <functional>
#include <ranges>
#include <string>
#include <vector>

namespace june {
namespace vm {

enum class Ops {
  // Create a new variable
  Create,
  // Store a value in a variable
  Store,

  // Load from operand onto the stack
  Load,
  // Pop from the stack
  Pop,

  // Unconditionally jump to `n`
  Jump,
  // Jump to `n` if top element on stack is true - will not pop if true
  JumpTrue,
  // Jump to `n` if top element on stack is false - will not pop if false
  JumpFalse,
  // Jump to `n` if top element on stack is true - will always pop
  JumpTruePop,
  // Jump to `n` if top element on stack is false - will always pop
  JumpFalsePop,
  // Jump to `n` if top element on stack is nil - will not pop otherwise
  JumpNil,

  // Jump to `n` where a body (of a function) ends + 1
  BodyMarker,
  // Make a function object
  MakeFunc,

  // Add count scopes
  BlkA,
  // Remove count scopes
  BlkR,

  // Call a function
  Call,
  // Call a member function
  CallMember,
  // Get attribute from an object
  Attr,

  // Return from a function
  Return,

  // Marks a loop beginning for a var stack
  PushLoop,
  // Marks a loop ending for a var stack
  PopLoop,

  // Continue a loop
  Continue,
  // Break a loop
  Break,

  // Marks the position to jump to if `or` exists in an expression
  PushJump,
  // Set the variable name for last jump instruction (after PushJump)
  PushJumpNamed,
  // Unmarks the position to jump to if `or` exists in an expression
  PopJump,

  _Count
};

extern const char *OpCodeStrs[(u64)Ops::_Count];

enum class OpDataType {
  Int,
  Float,
  String,
  Ident,
  Size,
  Bool,
  Nil,

  _Count
};

extern const char *OpDataTypeStrs[(u64)OpDataType::_Count];

struct NilOpData {};

union OpData {
  i64 sz;
  char *s;
  bool b;
  NilOpData n;
};

static auto opDataToString(const OpData d, const OpDataType t) -> std::string {
  switch (t) {
  case OpDataType::Int:
  case OpDataType::Size:
    return std::to_string(d.sz);
  case OpDataType::Float:
  case OpDataType::String:
  case OpDataType::Ident:
    return std::string(d.s);
  case OpDataType::Bool:
    return d.b ? "true" : "false";
  case OpDataType::Nil:
    return "nil";
  default:
    return "";
  }
}

inline static auto hashOpData(const OpDataType ty, const OpData d) -> int {
  switch (ty) {
  case OpDataType::Int:
  case OpDataType::Size:
    return std::hash<i64>()(d.sz);
  case OpDataType::Float:
  case OpDataType::String:
  case OpDataType::Ident:
    return std::hash<std::string>()(std::string(d.s));
  case OpDataType::Bool:
    return std::hash<bool>()(d.b);
  default:
    return 0;
  }
}

struct Op {
  u64 srcId;
  u64 idx;
  Ops op;
  u64 dataIdx;
  // OpDataType type;
  // OpData data;

  Op(const u64 &srcId, const u64 &idx, const Ops &op, const u64 &dataIdx)
      : srcId(srcId), idx(idx), op(op), dataIdx(dataIdx) {}

  auto str() const -> std::string;

  auto toBytes(OpDataType type) const -> std::vector<u8>;
};

using DataPair = std::pair<OpData, OpDataType>;

using namespace june::err;

struct Bytecode {
private:
  std::vector<Op> bytecode;
  std::unordered_map<u64, DataPair> data;

public:
  Bytecode() = default;
  ~Bytecode();

  /*
    [june bytecode version] - 8 bytes

    [data size] - 8 bytes (amount of data pairs of opdata and opdatatype)
    [data]

    [op size] - 8 bytes (amount of ops)
    [ops]
  */

  /*
    data format:

    [data type] - 1 byte

    if int/size:
      [data] - 8 bytes (unsigned, big endian)
    elif float/string/ident:
      [strlen] - 8 bytes (unsigned, big endian)
      [data] - [strlen] bytes
    elif bool:
      [data] - 1 byte
    elif nil:
      [data] - 0 bytes
  */

  /*
    op format:

    [op] - 2 bytes (unsigned, big endian)
    [src id] - 8 bytes (unsigned, big endian)
    [idx] - 8 bytes (unsigned, big endian)
    [data idx] - 8 bytes (unsigned, big endian)
  */

  // Inserts data or returns the index of data that already exists
  auto insertData(const OpDataType &type, const OpData &data) -> u64;

  auto addNil(const u64 &idx, const Ops op) -> void;
  auto addStr(const u64 &idx, const Ops op, const OpDataType type,
              const std::string &data) -> void;
  auto addBool(const u64 &idx, const Ops op, const bool &data) -> void;
  auto addSize(const u64 &idx, const Ops op, const u64 &data) -> void;
  auto addInt(const u64 &idx, const Ops op, const i64 &data) -> void;
  auto addFloat(const u64 &idx, const Ops op, const f64 &data) -> void;

  auto at(const u64 &idx) -> Op &;
  auto update(const u64 &idx, const Op &op) -> void;

  auto addOp(const Op op) -> void;

  auto toBytes() -> std::vector<u8>;
  static auto fromBytes(const std::vector<u8> &bytes) -> Result<Bytecode, Error>;
  
  auto str() const -> std::string;

  inline auto get() const -> const std::vector<Op> & { return bytecode; }
  inline auto getMut() -> std::vector<Op> & { return bytecode; }
  inline auto size() const -> u64 { return bytecode.size(); }
};

} // namespace vm
} // namespace june

#endif
