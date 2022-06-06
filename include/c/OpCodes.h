#ifndef vm_opcodes_h
#define vm_opcodes_h

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

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
  OpReturn,    // return from a function

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

static const char *OpCodeCStrs[_OpLast] = {
    "Create",        "Store",     "Load",        "Unload",       "Jump",
    "JumpTrue",      "JumpFalse", "JumpTruePop", "JumpFalsePop", "JumpNil",
    "BodyMarker",    "MakeFunc",  "BlkA",        "BlkR",         "Call",
    "MemberCall",    "Attr",  "Return",     "PushLoop",    "PopLoop", "Continue", "Break",      "PushJump",
    "PushJumpNamed", "PopJump"};

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

static const char *OpDataTypeCStrs[_OdtLast] = {
    "Int", "Float", "String", "Ident", "Size", "Bool", "Nil"};

union OpData {
  double f;
  long long i;
  size_t sz;
  char *s;
  bool b;
};

typedef struct Op {
  size_t srcId;
  size_t idx;
  enum OpCodes op;
  enum OpDataType type;
  union OpData data;
} Op;

const char *opAsString(Op op);

// struct Bytecode;
typedef void *BytecodeHandle;

BytecodeHandle BytecodeNew();
void BytecodeDelete(BytecodeHandle b);

void BytecodeAdd(BytecodeHandle b, const size_t idx, const OpCodes op);
void BytecodeAddString(BytecodeHandle b, const size_t idx, const OpCodes op,
                       const ::OpDataType type, const char *data);
void BytecodeAddIdent(BytecodeHandle b, const size_t idx, const OpCodes op,
                      const char *data);
void BytecodeAddSize(BytecodeHandle b, const size_t idx, const OpCodes op,
                     const size_t data);
void BytecodeAddBool(BytecodeHandle h, const size_t idx, const OpCodes op,
                     bool data);

OpCodes BytecodeGetOp(BytecodeHandle b, const size_t idx);
void BytecodeUpdateSize(BytecodeHandle b, const size_t idx, const size_t sz);

const Op **BytecodeGet(BytecodeHandle b);
Op **BytecodeGetMut(BytecodeHandle b);
size_t BytecodeGetSize(BytecodeHandle b);

#ifdef __cplusplus
}
#endif

#endif
