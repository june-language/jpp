#include "VM/OpCodes.hpp"
#include "VM/Memory.hpp"

namespace june {
namespace vm {

const char *OpCodeStrs[(u64)Ops::_Count] = {
    "Create",     "Store",     "Load",        "Pop",           "Jump",
    "JumpTrue",   "JumpFalse", "JumpTruePop", "JumpFalsePop",  "JumpNil",
    "BodyMarker", "MakeFunc",  "BlkA",        "BlkR",          "Call",
    "CallMember", "Attr",      "Return",      "PushLoop",      "PopLoop",
    "Continue",   "Break",     "PushJump",    "PushJumpNamed", "PopJump",
};

const char *OpDataTypeStrs[(u64)OpDataType::_Count] = {
    "Int", "Float", "String", "Ident", "Size", "Bool", "Nil"};

// Op

auto Op::str() const -> std::string {
  // Example:
  //
  // Create[0,0,0]

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

auto Bytecode::insertData(const OpDataType &type, const OpData &data_) -> u64 {
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
  data.n = NilOpData{};
  bytecode.emplace_back(idx, bytecode.size(), op,
                        insertData(OpDataType::Nil, data));
}

auto Bytecode::addStr(const u64 &idx, const Ops op, const OpDataType type,
                      const std::string &str) -> void {
  OpData data;
  data.s = const_cast<char *>(str.c_str());
  bytecode.emplace_back(idx, bytecode.size(), op, insertData(type, data));
}

auto Bytecode::addBool(const u64 &idx, const Ops op, const bool &data) -> void {
  OpData data_;
  data_.b = data;
  bytecode.emplace_back(idx, bytecode.size(), op,
                        insertData(OpDataType::Bool, data_));
}

auto Bytecode::addSize(const u64 &idx, const Ops op, const u64 &data) -> void {
  OpData data_;
  data_.sz = data;
  bytecode.emplace_back(idx, bytecode.size(), op,
                        insertData(OpDataType::Size, data_));
}

auto Bytecode::addInt(const u64 &idx, const Ops op, const i64 &data) -> void {
  OpData data_;
  data_.sz = data;
  bytecode.emplace_back(idx, bytecode.size(), op,
                        insertData(OpDataType::Int, data_));
}

auto Bytecode::addFloat(const u64 &idx, const Ops op, const f64 &data) -> void {
  OpData data_;
  data_.s = const_cast<char *>(fmt::format("{:f}", data).c_str());
  bytecode.emplace_back(idx, bytecode.size(), op,
                        insertData(OpDataType::Float, data_));
}

auto Bytecode::addOp(const Op op) -> void { bytecode.emplace_back(op); }

auto Bytecode::at(const u64 &idx) -> Op & { return bytecode[idx]; }

auto Bytecode::update(const u64 &idx, const Op &op) -> void {
  bytecode[idx] = op;
}

auto Op::toBytes(OpDataType type) const -> std::vector<u8> {
  /*
    op: 2 bytes
    srcId: 8 bytes
    idx: 8 bytes
    dataIdx: 8 bytes
  */

  std::vector<u8> bytes;

  auto opBytes =
      june::byteorder::toBytes(june::byteorder::swapBigEndian16((u16)op));
  bytes.insert(bytes.end(), opBytes.begin(), opBytes.end());

  auto srcIdBytes =
      june::byteorder::toBytes(june::byteorder::swapBigEndian64(srcId));
  bytes.insert(bytes.end(), srcIdBytes.begin(), srcIdBytes.end());

  auto idxBytes =
      june::byteorder::toBytes(june::byteorder::swapBigEndian64(idx));
  bytes.insert(bytes.end(), idxBytes.begin(), idxBytes.end());

  auto dataIdxBytes =
      june::byteorder::toBytes(june::byteorder::swapBigEndian64(dataIdx));
  bytes.insert(bytes.end(), dataIdxBytes.begin(), dataIdxBytes.end());

  return bytes;
}

auto Bytecode::toBytes() -> std::vector<u8> {
  /*
    [june bytecode version] - 8 bytes

    [data size] - 8 bytes (amount of data pairs of opdata and opdatatype)
    [data]

    [op size] - 8 bytes (amount of ops)
    [ops]
  */

  std::vector<u8> bytes;

  auto versionBytes = june::byteorder::toBytes(
      june::byteorder::swapBigEndian64((u64)JuneVersionNumber));
  bytes.insert(bytes.end(), versionBytes.begin(), versionBytes.end());

  auto dataSizeBytes =
      june::byteorder::toBytes(june::byteorder::swapBigEndian64(data.size()));
  bytes.insert(bytes.end(), dataSizeBytes.begin(), dataSizeBytes.end());

  auto values = june::structures::values(data);
  for (const auto &[data, dataType] : values) {
    std::vector<u8> dBytes;
    bool isString = false;
    bytes.push_back((u8)dataType);

    /*
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

    switch (dataType) {
    case OpDataType::Int:
    case OpDataType::Size:
      dBytes =
          june::byteorder::toBytes(june::byteorder::swapBigEndian64(data.sz));
      break;
    case OpDataType::Float:
    case OpDataType::String:
    case OpDataType::Ident:
      // append strlen before string
      // dBytes =
      // june::byteorder::toBytes(june::byteorder::swapBigEndian64(strlen(data.s)));
      isString = true;
      dBytes = june::byteorder::toBytes((const std::string &)data.s);
      break;
    case OpDataType::Bool:
      dBytes.push_back((u8)data.b);
      break;
    case OpDataType::Nil:
      break;
    default:
      JUNE_UNREACHABLE;
    }

    if (isString) {
      auto lenBytes = june::byteorder::toBytes(
          june::byteorder::swapBigEndian64(strlen(data.s)));
      dBytes.insert(dBytes.begin(), lenBytes.begin(), lenBytes.end());
    }

    bytes.insert(bytes.end(), dBytes.begin(), dBytes.end());
  }

  auto opCountBytes = june::byteorder::toBytes(
      june::byteorder::swapBigEndian64(bytecode.size()));
  bytes.insert(bytes.end(), opCountBytes.begin(), opCountBytes.end());

  for (const auto &op : bytecode) {
    auto opBytes = op.toBytes(data[op.dataIdx].second);
    bytes.insert(bytes.end(), opBytes.begin(), opBytes.end());
  }

  return bytes;
}

auto Bytecode::fromBytes(const std::vector<u8> &bytes)
    -> Result<Bytecode, Error> {
  Bytecode bytecode;
  june::byteorder::ByteReader reader(bytes);

  if (june::byteorder::swapBigEndian64(reader.read<u64>()) !=
      JuneVersionNumber) {
    return bws::fail(Error(ErrorKind::FileIo, "Invalid bytecode version"));
  }

  auto dataSize = june::byteorder::swapBigEndian64(reader.read<u64>());
  for (u64 i = 0; i < dataSize; i++) {
    auto dataType = (OpDataType)reader.read<u8>();
    u64 stringLength = 0; // for string, ident, float

    switch (dataType) {
    case OpDataType::Int:
    case OpDataType::Size:
      bytecode.insertData(dataType,
                          OpData{.sz = june::byteorder::swapBigEndian64Signed(
                                     reader.read<i64>())});
      break;
    case OpDataType::Float:
    case OpDataType::String:
    case OpDataType::Ident:
      stringLength = june::byteorder::swapBigEndian64(reader.read<u64>());
      bytecode.insertData(dataType, OpData{.s = (char*)reader.readString(stringLength)});
      break;
    case OpDataType::Bool:
      bytecode.insertData(dataType, OpData{.b = (bool)reader.read<u8>()});
      break;
    case OpDataType::Nil:
      bytecode.insertData(dataType,
                          OpData{.n = NilOpData{}}); // it'll insert once and never again
      break;
    default:
      JUNE_UNREACHABLE;
    }
  }

  auto opCount = june::byteorder::swapBigEndian64(reader.read<u64>());
  for (u64 i = 0; i < opCount; i++) {
    auto opcode = june::byteorder::swapBigEndian16(reader.read<u16>());
    auto srcId = june::byteorder::swapBigEndian64(reader.read<u64>());
    auto idx = june::byteorder::swapBigEndian64(reader.read<u64>());
    auto dataIdx = june::byteorder::swapBigEndian64(reader.read<u64>());

    auto op = Op(srcId, idx, (Ops)opcode, dataIdx);
    bytecode.addOp(op);
  }

  return bytecode;
}

auto Bytecode::str() const -> std::string {
  std::string s = "Bytecode:\n";
  s += "  Data:\n";
  for (const auto &[_, dataPair] : data) {
    auto [data, dataType] = dataPair;
    s += "    " + std::string(OpDataTypeStrs[(u8)dataType]) + "[" +
         opDataToString(data, dataType) + "]\n";
  }

  s += "  Ops:\n";
  for (const auto &op : bytecode) {
    s += "    " + op.str() + "\n";
  }
  return s;
}

} // namespace vm
} // namespace june
