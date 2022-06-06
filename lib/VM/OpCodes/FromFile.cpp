#include "Common.hpp"
#include "VM/OpCodes.hpp"
#include <cstring>
#include <unordered_map>

using namespace june;

int hash(const OpDataType type, const OpData data) {
  switch (type) {
  case OdtSize:
    return std::hash<size_t>()(data.sz);
  case OdtString:
  case OdtIdent:
    return std::hash<std::string>()(data.s);
  case OdtBool:
    return std::hash<bool>()(data.b);
  default:
    return 0;
  }
}

FileCompatibleBytecode june::compressBytecode(const std::vector<Op> &bytecode) {
  std::unordered_map<int, std::pair<OpData, OpDataType>> compressedData;
  FileCompatibleBytecode fcb;
  fcb.bytecode.reserve(bytecode.size());

  for (auto &op : bytecode) {
    FileCompatibleOp fco;
    fco.srcId = op.srcId;
    fco.idx = op.idx;
    fco.op = op.op;
    fco.type = op.type;

    int dataHash = hash(op.type, op.data);
    auto it = compressedData.find(dataHash);
    if (it == compressedData.end()) {
      compressedData[dataHash] = {op.data, op.type};
      fco.dataIndex = dataHash;
    } else {
      fco.dataIndex = it->first;
    }

    fcb.bytecode.push_back(fco);
  }

  return fcb;
}

DecompressResult
june::decompressBytecode(const FileCompatibleBytecode &bytecode) {
  std::vector<Op> decompressedBytecode;
  std::unordered_map<int, OpData> compressedData;

  for (auto &data : bytecode.compressedData) {
    compressedData[hash(data.second, data.first)] = data.first;
  }

  for (auto &op : bytecode.bytecode) {
    Op opd;
    opd.srcId = op.srcId;
    opd.idx = op.idx;
    opd.op = op.op;
    opd.type = op.type;

    auto it = compressedData.find(op.dataIndex);
    if (it == compressedData.end()) {
      return DecompressResult::Err(
          "Invalid compressed bytecode, data index not found");
    } else {
      opd.data = it->second;
    }

    decompressedBytecode.push_back(opd);
  }

  return DecompressResult::Ok(decompressedBytecode);
}

namespace june {
namespace fs {

using namespace june::fs;

u8 *writeBytecode(const std::vector<Op> &bytecode,
                  const std::vector<SrcColRange> &srcRanges) {
  /**
   * The format of the file
   *
   * bytecode:
   *
   * [magic]
   * [col count (u32)]
   * [cols]
   * [data size (u32)]
   * [data]
   * [op size (u32)]
   * [ops]
   *
   * magic:
   *
   * 'J' 'U' 'N' 'E'
   * 0x4A, 0x55, 0x4E, 0x45
   *
   * cols:
   * [
   *
   * data:
   *
   * [data type (u8)]
   * [inner (size depends on data type, see below)]
   *
   * inner:
   * if data type is string, ident:
   * [str size (u32)]
   * [str]
   * if data type is size:
   * [size (u64)]
   * if data type is int:
   * [int (i64)]
   * if data type is float:
   * [float (f64)]
   * if data type is bool:
   * [bool (u8)]
   *
   * ops:
   *
   * [src id len (u32)]
   * [src id]
   * [idx (u32)]
   * [op (u8)]
   * [type (u8)]
   * [data index (u32)]
   */

  u8 *data = nullptr;
  size_t dataSize = 0;
  size_t opSize = 0;

  auto compressedBytecode = compressBytecode(bytecode);

  for (auto &d : compressedBytecode.compressedData) {
    dataSize += sizeof(u8);
    switch (d.second) {
    case OdtString:
    case OdtIdent:
      dataSize += sizeof(u32);
      dataSize += strlen(d.first.s);
      break;
    case OdtSize:
    case OdtInt:
    case OdtFloat:
      dataSize += sizeof(u64);
      break;
    case OdtBool:
      dataSize += sizeof(u8);
      break;
    default:
      break;
    }
  }

  for (auto &op : compressedBytecode.bytecode) {
    opSize += sizeof(u32);
    opSize += sizeof(u64);
    opSize += sizeof(u32);
    opSize += sizeof(u8);
    opSize += sizeof(u8);
    opSize += sizeof(u32);
  }

  data = new u8[dataSize + opSize + sizeof(u32) * 2 + sizeof(u8) * 4];

  data[0] = 'J';
  data[1] = 'U';
  data[2] = 'N';
  data[3] = 'E';

  u32 colCount = srcRanges.size();
  memcpy(data + 4, &colCount, sizeof(u32));

  for (u32 i = 0; i < colCount; i++) {
    u64 colBegin = srcRanges[i].begin;
    u64 colEnd = srcRanges[i].end;
    memcpy(data + 8 + i * 8, &colBegin, sizeof(u64));
    memcpy(data + 8 + i * 8 + 4, &colEnd, sizeof(u64));
  }

  u32 dataSizeU32 = dataSize;
  memcpy(data + 8 + (colCount * (sizeof(u64) * 2)), &dataSizeU32, sizeof(u32));

  u8 offset = 8 + (colCount * (sizeof(u64) * 2)) + sizeof(u32);
  for (auto &d : compressedBytecode.compressedData) {
    u8 type = d.second;
    memcpy(data + offset, &type, sizeof(u8));
    offset += sizeof(u8);

    switch (d.second) {
    case OdtFloat:
    case OdtInt:
    case OdtString:
    case OdtIdent: {
      u32 strSize = strlen(d.first.s);
      memcpy(data + offset, &strSize, sizeof(u32));
      offset += sizeof(u32);
      memcpy(data + offset, d.first.s, strSize);
      offset += strSize;
      break;
    }
    case OdtSize: {
      u64 size = d.first.sz;
      memcpy(data + offset, &size, sizeof(u64));
      offset += sizeof(u64);
      break;
    }
    case OdtBool: {
      u8 b = d.first.b;
      memcpy(data + offset, &b, sizeof(u8));
      offset += sizeof(u8);
      break;
    }
    default:
      break;
    }
  }

  u32 opSizeU32 = opSize;
  memcpy(data + offset, &opSizeU32, sizeof(u32));
  offset += sizeof(u32);

  for (auto &op : compressedBytecode.bytecode) {
    memcpy(data + offset, &op.srcId, sizeof(u64));
    offset += sizeof(u64);
    memcpy(data + offset, &op.idx, sizeof(u32));
    offset += sizeof(u32);
    memcpy(data + offset, &op.op, sizeof(u8));
    offset += sizeof(u8);
    memcpy(data + offset, &op.type, sizeof(u8));
    offset += sizeof(u8);
    memcpy(data + offset, &op.dataIndex, sizeof(u32));
    offset += sizeof(u32);
  }

  return data;
}

ReadResult readBytecode(const u8 *bytecode) {
  /**
   * The format of the file
   *
   * bytecode:
   *
   * [magic]
   * [col count (u32)]
   * [cols]
   * [data size (u32)]
   * [data]
   * [op size (u32)]
   * [ops]
   *
   * magic:
   *
   * 'J' 'U' 'N' 'E'
   * 0x4A, 0x55, 0x4E, 0x45
   *
   * data:
   *
   * [data type (u8)]
   * [inner (size depends on data type, see below)]
   *
   * inner:
   * if data type is string, ident:
   * [str size (u32)]
   * [str]
   * if data type is size:
   * [size (u64)]
   * if data type is int:
   * [int (i64)]
   * if data type is float:
   * [float (f64)]
   * if data type is bool:
   * [bool (u8)]
   *
   * ops:
   *
   * [src id len (u32)]
   * [src id]
   * [idx (u32)]
   * [op (u8)]
   * [type (u8)]
   * [data index (u32)]
   */

  if (bytecode[0] != 'J' || bytecode[1] != 'U' || bytecode[2] != 'N' ||
      bytecode[3] != 'E') {
    return ReadResult::Err("Invalid bytecode, invalid magic");
  }

  u32 colCount;
  u32 colSize;
  u32 dataSize;
  u32 opSize;

  memcpy(&colCount, bytecode + 4, sizeof(u32));
  colSize = colCount * (sizeof(u64) * 2);

  memcpy(&dataSize, bytecode + 8 + colSize, sizeof(u32));
  memcpy(&opSize, bytecode + 8 + colSize + 4 + dataSize, sizeof(u32));

  std::vector<std::pair<OpData, OpDataType>> compressedData;
  std::vector<FileCompatibleOp> bytecodeOps;
  std::vector<SrcColRange> srcRanges;

  u8 offset = 8; // magic + colCount
  for (u32 i = 0; i < colCount; i++) {
    u64 colBegin;
    u64 colEnd;
    memcpy(&colBegin, bytecode + offset, sizeof(u64));
    offset += sizeof(u64);
    memcpy(&colEnd, bytecode + offset, sizeof(u64));
    offset += sizeof(u64);
    srcRanges.push_back({colBegin, colEnd});
  }

  offset += sizeof(u32); // skip data size
  for (u32 i = 0; i < dataSize;) {
    u8 type;
    memcpy(&type, bytecode + offset, sizeof(u8));
    offset += sizeof(u8);
    i += sizeof(u8);

    switch (type) {
    case OdtInt:
    case OdtFloat:
    case OdtString:
    case OdtIdent: {
      u32 strSize;
      memcpy(&strSize, bytecode + offset, sizeof(u32));
      offset += sizeof(u32);

      char *str = new char[strSize + 1];
      memcpy(str, bytecode + offset, strSize);
      str[strSize] = '\0';
      offset += strSize;
      i += strSize + sizeof(u32);

      compressedData.push_back({{.s = str}, static_cast<OpDataType>(type)});
      break;
    }
    case OdtSize: {
      u64 size;
      memcpy(&size, bytecode + offset, sizeof(u64));
      offset += sizeof(u64);
      i += sizeof(u64);

      compressedData.push_back({{.sz = size}, static_cast<OpDataType>(type)});
      break;
    }
    case OdtBool: {
      u8 b;
      memcpy(&b, bytecode + offset, sizeof(u8));
      offset += sizeof(u8);
      i += sizeof(u8);
      compressedData.push_back(
          {{.b = static_cast<bool>(b)}, static_cast<OpDataType>(type)});
      break;
    }
    default:
      break;
    }
  }

  offset += sizeof(u32); // skip op size
  for (u32 i = 0; i < opSize;) {
    u64 srcId;
    memcpy(&srcId, bytecode + offset, sizeof(u64));
    offset += sizeof(u64);
    i += sizeof(u64);

    u32 idx;
    memcpy(&idx, bytecode + offset, sizeof(u32));
    offset += sizeof(u32);
    i += sizeof(u32);

    u8 op;
    memcpy(&op, bytecode + offset, sizeof(u8));
    offset += sizeof(u8);
    i += sizeof(u8);

    u8 type;
    memcpy(&type, bytecode + offset, sizeof(u8));
    offset += sizeof(u8);
    i += sizeof(u8);

    u32 dataIndex;
    memcpy(&dataIndex, bytecode + offset, sizeof(u32));
    offset += sizeof(u32);
    i += sizeof(u32);

    bytecodeOps.push_back(FileCompatibleOp{srcId, idx, static_cast<OpCodes>(op),
                                           static_cast<OpDataType>(type),
                                           dataIndex});
  }

  auto res =
      decompressBytecode(FileCompatibleBytecode{compressedData, bytecodeOps});

  if (res.isErr()) {
    return ReadResult::Err(res.unwrapErr());
  }

  return ReadResult::Ok({.bytecode = res.unwrap(), .srcRanges = {}});
}

} // namespace fs
} // namespace june
