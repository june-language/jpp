#include "VM/OpCodes.hpp"
#include "Common.hpp"
#include "VM/Memory.hpp"
#include "c/OpCodes.h"
#include <sstream>
#include <string>

using namespace june;

const char *june::OpCodeStrs[_OpLast] = {
    "Create",     "Store",     "Load",          "Unload",       "Jump",
    "JumpTrue",   "JumpFalse", "JumpTruePop",   "JumpFalsePop", "JumpNil",
    "BodyMarker", "MakeFunc",  "BlkA",          "BlkR",         "Call",
    "MemberCall", "Attr",      "Return",        "PushLoop",     "PopLoop",
    "Continue",   "PushJump",  "PushJumpNamed", "PopJump",
};

const char *june::OpDataTypeStrs[_OdtLast] = {
    "Int", "Float", "String", "Ident", "Size", "Bool", "Nil",
};

std::string june::opAsString(Op op) {
  std::stringstream ss;

  // <srcId>:<idx>:<op_code> <data_type> <data>
  ss << op.srcId << ":" << op.idx << ":" << OpCodeStrs[op.op] << " "
     << OpDataTypeStrs[op.type] << " ";

  switch (op.type) {
  case OdtInt:
    ss << op.data.sz;
    break;
  case OdtFloat:
    ss << op.data.sz;
    break;
  case OdtString:
    ss << op.data.s;
    break;
  case OdtIdent:
    ss << op.data.s;
    break;
  case OdtSize:
    ss << op.data.sz;
    break;
  case OdtBool:
    ss << op.data.b;
    break;
  case OdtNil:
    ss << "nil";
    break;
  default:
    break;
  }

  return ss.str();
}

june::Bytecode::~Bytecode() {
  using namespace june::mem;
  
  for (auto &op : bytecode) {
    if (op.type != OdtSize && op.type != OdtBool && op.type != OdtNil) {
      mem::free(op.data.s,
                      mem::mult8_roundup(strlen(op.data.s) + 1));
    }
  }
}

void june::Bytecode::add(const size_t &idx, const OpCodes op) {
  this->bytecode.push_back(Op{0, idx, op, OdtNil, {.s = nullptr}});
}

void june::Bytecode::adds(const size_t &idx, const OpCodes op,
                          const OpDataType dtype, const std::string &data) {
  this->bytecode.push_back(
      Op{0,
         idx,
         op,
         dtype,
         {.s = (char *)june::string::duplicateAsCString(data)}});
}

void june::Bytecode::addb(const size_t &idx, const OpCodes op,
                          const bool &data) {
  this->bytecode.push_back(Op{0, idx, op, OdtBool, {.b = data}});
}

void june::Bytecode::addsz(const size_t &idx, const OpCodes op,
                           const size_t &data) {
  this->bytecode.push_back(Op{0, idx, op, OdtSize, {.sz = data}});
}

june::OpCodes june::Bytecode::at(const size_t &pos) const {
  return pos >= bytecode.size() ? _OpLast : this->bytecode.at(pos).op;
}

void june::Bytecode::updatesz(const size_t &pos, const size_t &value) {
  if (pos >= bytecode.size())
    return;
  this->bytecode.at(pos).data.sz = value;
}

// C API

june::OpCodes COpCodeToOpCode(const ::OpCodes op) {
  return static_cast<june::OpCodes>(op);
}

::OpCodes OpCodeToCOpCode(const june::OpCodes op) {
  return static_cast<::OpCodes>(op);
}

::OpData OpDataToCOpData(const june::OpData &data,
                         const june::OpDataType type) {
  ::OpData opData;
  switch (type) {
  case june::OdtInt:
  case june::OdtFloat:
  case june::OdtString:
    opData.s = (char *)june::string::duplicateAsCString(data.s);
    break;
  case june::OdtIdent:
    opData.s = (char *)june::string::duplicateAsCString(data.s);
    break;
  case june::OdtSize:
    opData.sz = data.sz;
    break;
  case june::OdtBool:
    opData.b = data.b;
    break;
  case june::OdtNil:
    opData.s = nullptr;
    break;
  default:
    break;
  }
  return opData;
}

june::OpData COpDataToOpData(const ::OpData &data, const ::OpDataType type) {
  june::OpData opData;
  switch (type) {
  case ::OdtInt:
  case ::OdtFloat:
  case ::OdtString:
    opData.s = (char *)june::string::duplicateAsCString(data.s);
    break;
  case ::OdtIdent:
    opData.s = (char *)june::string::duplicateAsCString(data.s);
    break;
  case ::OdtSize:
    opData.sz = data.sz;
    break;
  case ::OdtBool:
    opData.b = data.b;
    break;
  case ::OdtNil:
    opData.s = nullptr;
    break;
  default:
    break;
  }
  return opData;
}

::OpDataType OpDataTypeToCOpDataType(const june::OpDataType type) {
  return static_cast<::OpDataType>(type);
}

june::OpDataType COpDataTypeToOpDataType(const ::OpDataType type) {
  return static_cast<june::OpDataType>(type);
}

::Op *OpToCOp(const june::Op op) {
  ::Op *op_ = new ::Op;
  op_->srcId = op.srcId;
  op_->idx = op.idx;
  op_->op = OpCodeToCOpCode(op.op);
  op_->type = OpDataTypeToCOpDataType(op.type);
  op_->data = OpDataToCOpData(op.data, op.type);
  return op_;
}

june::Op COpToOp(const ::Op *op) {
  return june::Op{op->srcId, op->idx, COpCodeToOpCode(op->op),
                  COpDataTypeToOpDataType(op->type),
                  COpDataToOpData(op->data, op->type)};
}

june::Bytecode *BytecodeFromC(const ::BytecodeHandle c_bc) {
  return reinterpret_cast<june::Bytecode *>(c_bc);
}

extern "C" const char *opAsString(::Op op) {
  return june::string::duplicateAsCString(june::opAsString(COpToOp(&op)));
}

extern "C" BytecodeHandle BytecodeNew() { return new june::Bytecode(); }

extern "C" void BytecodeDelete(BytecodeHandle bc) { delete BytecodeFromC(bc); }

extern "C" void BytecodeAdd(BytecodeHandle b, const size_t idx,
                            const ::OpCodes op) {
  BytecodeFromC(b)->add(idx, COpCodeToOpCode(op));
}

extern "C" void BytecodeAddString(BytecodeHandle b, const size_t idx,
                                  const ::OpCodes op, const ::OpDataType type,
                                  const char *data) {
  BytecodeFromC(b)->adds(idx, COpCodeToOpCode(op),
                         COpDataTypeToOpDataType(type), data);
}

extern "C" void BytecodeAddIdent(BytecodeHandle b, const size_t idx,
                                 const ::OpCodes op, const char *data) {
  BytecodeFromC(b)->adds(idx, COpCodeToOpCode(op), june::OdtIdent, data);
}

extern "C" void BytecodeAddSize(BytecodeHandle b, const size_t idx,
                                const ::OpCodes op, const size_t data) {
  BytecodeFromC(b)->addsz(idx, COpCodeToOpCode(op), data);
}

extern "C" void BytecodeAddBool(BytecodeHandle h, const size_t idx,
                                const ::OpCodes op, bool data) {
  BytecodeFromC(h)->addb(idx, COpCodeToOpCode(op), data);
}

extern "C" ::OpCodes BytecodeGetOp(BytecodeHandle b, const size_t idx) {
  return OpCodeToCOpCode(BytecodeFromC(b)->at(idx));
}

extern "C" void BytecodeUpdateSize(BytecodeHandle b, const size_t idx,
                                   const size_t value) {
  BytecodeFromC(b)->updatesz(idx, value);
}

extern "C" const ::Op **BytecodeGet(BytecodeHandle b) {
  const ::Op **ops = new const ::Op *[BytecodeFromC(b)->size()];
  for (size_t i = 0; i < BytecodeFromC(b)->size(); i++) {
    ops[i] = OpToCOp(BytecodeFromC(b)->get()[i]);
  }

  return ops;
}

extern "C" ::Op **BytecodeGetMut(BytecodeHandle b) {
  ::Op **ops = new ::Op *[BytecodeFromC(b)->size()];
  for (size_t i = 0; i < BytecodeFromC(b)->size(); i++) {
    ops[i] = OpToCOp(BytecodeFromC(b)->get()[i]);
  }

  return ops;
}

extern "C" size_t BytecodeGetSize(BytecodeHandle b) {
  return BytecodeFromC(b)->size();
}
