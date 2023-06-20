#include "VM/OpCodes.hpp"
#include "VM/Memory.hpp"

namespace june {
namespace vm {

const char *OpCodeStrs[(u64)Ops::_Count] = {
    "Create",       "Store",    "Load",       "Pop",
    "Jump",         "JumpTrue", "JumpFalse",  "JumpTruePop",
    "JumpFalsePop", "JumpNil",  "BodyMarker", "MakeFunc",
    "BlkA",         "BlkR",     "Call",       "CallMember",
    "Attr",         "Return",   "PushLoop",   "PopLoop",
    "Continue",     "Break",    "PushJump",   "PushJumpNamed",
    "PopJump",
};

const char *OpDataTypeStrs[(u64)OpDataType::_Count] = {
    "Int",  "Float", "String", "Ident",
    "Size", "Bool",  "Nil"};

// Op

auto Op::str() const -> std::string {
  // Example:
  //
  // OpCreate[0,0,0]

  // <op>[<srcId>,<idx>,<dataIdx>]

  std::string str = OpCodeStrs[(u64)op];

  return str + "[" + std::to_string(srcId) + "," + std::to_string(idx) + "," +
         std::to_string(dataIdx) + "]";
}

// Bytecode

Bytecode::~Bytecode() {
  using namespace june::mem;

  for (auto &op : bytecode) {
    auto data = this->data.find(op.dataIdx);
    if (data != this->data.end()) {
      if (data->second.second != OpDataType::Size &&
          data->second.second != OpDataType::Bool &&
          data->second.second != OpDataType::Nil) {
        mem::free(data->second.first.s,
                  mem::mult8RoundUp(strlen(data->second.first.s) + 1));
      }
    }
  }
}

auto Bytecode::insertData(const OpDataType &type, const OpData &data_)
    -> u64 {
  auto hash = june::vm::hashOpData(type, data_);
  auto it = data.find(hash);

  if (it != data.end()) {
    return it->first;
  }

  data.emplace(hash, std::pair<OpData, OpDataType>(data_, type));
  return hash;
}

auto Bytecode::addNil(const u64 &idx, const Ops op) -> void {
  OpData data;
  data.b = false;
  bytecode.emplace_back(idx, bytecode.size(), op,
                        insertData(OpDataType::Nil, data));
}

auto Bytecode::addStr(const u64 &idx, const Ops op,
                      const OpDataType type, const std::string &str) -> void {
  OpData data;
  data.s = const_cast<char*>(str.c_str());
  bytecode.emplace_back(idx, bytecode.size(), op, insertData(type, data));
}

auto Bytecode::addBool(const u64 &idx, const Ops op, const bool &data)
    -> void {
  OpData data_;
  data_.b = data;
  bytecode.emplace_back(idx, bytecode.size(), op,
                        insertData(OpDataType::Bool, data_));
}

auto Bytecode::addSize(const u64 &idx, const Ops op, const u64 &data)
    -> void {
  OpData data_;
  data_.sz = data;
  bytecode.emplace_back(idx, bytecode.size(), op,
                        insertData(OpDataType::Size, data_));
}

auto Bytecode::addInt(const u64 &idx, const Ops op, const int &data)
    -> void {
  OpData data_;
  data_.sz = data;
  bytecode.emplace_back(idx, bytecode.size(), op,
                        insertData(OpDataType::Int, data_));
}

auto Bytecode::addFloat(const u64 &idx, const Ops op, const float &data)
    -> void {
  OpData data_;
  data_.s = const_cast<char*>(fmt::format("{:f}", data).c_str());
  bytecode.emplace_back(idx, bytecode.size(), op,
                        insertData(OpDataType::Float, data_));
}

auto Bytecode::at(const u64 &idx) -> Op & { return bytecode[idx]; }

auto Bytecode::update(const u64 &idx, const Op &op) -> void {
  bytecode[idx] = op;
}

auto Op::toBytes() const -> std::vector<u8> {
  /*
    op: 2 bytes
    srcId: 8 bytes
    idx: 8 bytes
    dataIdx: 8 bytes
  */

  std::vector<u8> bytes;

  auto opBytes = june::byteorder::toBytes(june::byteorder::toBigEndian16((u16)op));
  bytes.insert(bytes.end(), opBytes.begin(), opBytes.end());

  auto srcIdBytes = june::byteorder::toBytes(june::byteorder::toBigEndian64(srcId));
  bytes.insert(bytes.end(), srcIdBytes.begin(), srcIdBytes.end());

  auto idxBytes = june::byteorder::toBytes(june::byteorder::toBigEndian64(idx));
  bytes.insert(bytes.end(), idxBytes.begin(), idxBytes.end());

  auto dataIdxBytes = june::byteorder::toBytes(june::byteorder::toBigEndian64(dataIdx));
  bytes.insert(bytes.end(), dataIdxBytes.begin(), dataIdxBytes.end());

  return bytes;
}

auto Bytecode::toBytes() -> std::vector<u8> {
  /*
    [june bytecode version] - 8 bytes

    [data size] - 8 bytes (amount of data pairs of opdata and opdatatype)
    [data]

    [data type] - 1 byte (opdatatype)

    [op size] - 8 bytes (amount of ops)
    [ops]
  */

  std::vector<u8> bytes;

  auto versionBytes = june::byteorder::toBytes(june::byteorder::toBigEndian16((u64)JuneVersionNumber));
  bytes.insert(bytes.end(), versionBytes.begin(), versionBytes.end());

  auto dataSizeBytes = june::byteorder::toBytes(june::byteorder::toBigEndian64(data.size()));
  bytes.insert(bytes.end(), dataSizeBytes.begin(), dataSizeBytes.end());

  auto values = june::structures::values(data);
  for (const auto &[data, dataType] : values) {
    std::vector<u8> dBytes;
    bytes.push_back((u8)dataType);
    switch (dataType) {
    case OpDataType::Int:
    case OpDataType::Size:
      dBytes = june::byteorder::toBytes(june::byteorder::toBigEndian64(data.sz));
      break;
    case OpDataType::Float:
    case OpDataType::String:
    case OpDataType::Ident:
      dBytes = june::byteorder::toBytes(june::byteorder::toBigEndian64((u64)data.s));
      break;
    case OpDataType::Bool:
      dBytes.push_back((u8)data.b);
      break;
    default:
      JUNE_UNREACHABLE;
    }

    if (bytes.back() == 1) {
      // append strlen before string
      auto strLenBytes = june::byteorder::toBytes(june::byteorder::toBigEndian64(strlen(data.s)));
      bytes.insert(bytes.end(), strLenBytes.begin(), strLenBytes.end());
    }

    bytes.insert(bytes.end(), dBytes.begin(), dBytes.end());
  }

  auto opCountBytes = june::byteorder::toBytes(june::byteorder::toBigEndian64(bytecode.size()));
  bytes.insert(bytes.end(), opCountBytes.begin(), opCountBytes.end());

  for (const auto &op : bytecode) {
    auto opBytes = op.toBytes();
    bytes.insert(bytes.end(), opBytes.begin(), opBytes.end());
  }

  return bytes;
}

} // namespace vm
} // namespace june
