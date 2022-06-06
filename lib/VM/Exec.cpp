#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <unordered_map>

#include "Common.hpp"
#include "JuneConfig.hpp"
#include "VM/Consts.hpp"
#include "VM/OpCodes.hpp"
#include "VM/State.hpp"
#include "VM/Vars.hpp"
#include "VM/Vars/Base.hpp"
#include "c/OpCodes.h"

namespace june {

struct JumpData {
  const char *name;
  size_t pos;
};

using namespace err;

namespace vm {

char *execFailFmt(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *msg;
  vasprintf(&msg, fmt, args);
  va_end(args);
  return msg;
}

#define execFail(failure, ...)                                                 \
  do {                                                                         \
    handleError(vm, jumps, vars, op, i);                                       \
    if (!customBytecode)                                                       \
      vars->popFn();                                                           \
    vm.execStackCount--;                                                       \
    return Error(ErrExecFail, execFailFmt(failure, ##__VA_ARGS__));            \
  } while (0)

void handleError(State &vm, std::vector<JumpData> &jumps, Vars *vars,
                 const Op &op, size_t &i) {
  if (!jumps.empty() && !vm.exitCalled) {
    i = jumps.back().pos - 1;
    if (jumps.back().name) {
      if (!vm.fails.backEmpty()) {
        vars->stash(jumps.back().name, vm.fails.pop(false), false);
      } else {
        vars->stash(jumps.back().name,
                    make_all<VarString>("Unknown failure", op.srcId, op.idx));
      }
    }
    jumps.pop_back();
    vm.fails.blkr();
    vm.execStackCountExceeded = false;
  }
}

ExecResult exec(State &vm, const Bytecode *customBytecode, const size_t &begin,
                const size_t &end) {
  vm.execStackCount++;

  VarSrc *src = vm.currentSource();
  Vars *vars = src->vars();
  SrcFile *srcFile = src->src();
  size_t srcId = srcFile->id();
  Stack *vms = vm.stack;
  const auto &bc =
      customBytecode ? customBytecode->get() : srcFile->bytecode().get();
  size_t bytecodeSize = end == 0 ? bc.size() : end;

  std::vector<FnBodySpan> bodies;
  std::vector<VarBase *> args;
  std::vector<JumpData> jumps;

  if (!customBytecode)
    vars->pushFn();

  for (size_t i = begin; i < bytecodeSize; i++) {
    const Op &op = bc[i];
    if (vm.execStackCount >= vm.execStackMax) {
      vm.fail(bc[i].srcId, bc[i].idx,
              "exceeded call stack size, currently: %zu", vm.execStackCount);
      vm.execStackCountExceeded = true;
      execFail("exceeded call stack size");
    }

    if (JuneDebug) {
      printf("%s [%zu] : %*s: ", srcFile->path().c_str(), i, 12,
             OpCodeStrs[op.op]);

      for (auto &e : vms->get()) {
        printf("%s ", vm.getTypeName(e).c_str());
      }

      printf("\n");
    }

    switch (op.op) {
    case OpLoad: {
      if (op.type != OdtIdent) {
        VarBase *res = constants::get(vm, op.type, op.data, op.srcId, op.idx);
        if (res == nullptr) {
          vm.fail(op.srcId, op.idx, "invalid data recieved as a constant");
          execFail("invalid data recieved as a constant");
        }
        vms->push(res);
      } else {
        VarBase *res = vars->get(op.data.s);
        if (res == nullptr) {
          res = vm.globalGet(op.data.s);
          if (res == nullptr) {
            vm.fail(op.srcId, op.idx, "variable '%s' does not exist",
                    op.data.s);
            execFail("variable '%s' does not exist", op.data.s);
          }
        }
        vms->push(res, true);
      }
      break;
    }
    case OpUnload: {
      vms->pop();
      break;
    }
    case OpCreate: {
      const std::string name = vms->back()->as<VarString>()->get();
      vms->pop();
      VarBase *ctx = nullptr;
      if (op.data.b) {
        ctx = vms->pop(false);
      }
      VarBase *val = vms->pop(false);
      if (!ctx) {
        if (val->isLoadAsRef() || val->refCount() == 1) {
          vars->add(name, val, true);
          val->unsetLoadAsRef();
        } else {
          vars->add(name, val->copy(op.srcId, op.idx), false);
        }
        varDref(val);
        break;
      }

      if (ctx->isAttrBased()) {
        if (val->isLoadAsRef() || val->refCount() == 1) {
          ctx->attrSet(name, val, true);
          val->unsetLoadAsRef();
        } else {
          ctx->attrSet(name, val->copy(op.srcId, op.idx), false);
        }
      }

      if (!val->isCallable()) {
        varDref(ctx);
        varDref(val);
        vm.fail(
            op.srcId, op.idx,
            "only callable values can be added to non-attribute based types");
        execFail(
            "only callable values can be added to non-attribute based types");
      }

      vm.addTypeFn(ctx->isa<VarTypeId>() ? ctx->as<VarTypeId>()->get()
                                         : ctx->typeFnId(),
                   name, val, true);
      varDref(ctx);
      varDref(val);
      break;
    }
    case OpStore: {
      if (vms->size() < 2) {
        vm.fail(op.srcId, op.idx,
                "vm stack has %zu elements, expected at least "
                "2",
                vms->size());
        execFail("vm stack has %zu elements, expected at least 2", vms->size());
      }

      VarBase *var = vms->pop(false);
      VarBase *val = vms->pop(false);
      if (var->type() != val->type()) {
        varDref(val);
        varDref(var);
        vm.fail(op.srcId, op.idx,
                "type mismatch: %s cannot be assigned to variable "
                "of type %s",
                vm.getTypeName(var).c_str(), vm.getTypeName(val).c_str());
        execFail("type mismatch: %s cannot be assigned to variable of type %s",
                 vm.getTypeName(var).c_str(), vm.getTypeName(val).c_str());
      }

      var->set(val);
      vms->push(var, false);
      varDref(val);
      break;
    }
    case OpBlkA: {
      vars->blkAdd(op.data.sz);
      break;
    }
    case OpBlkR: {
      vars->blkRem(op.data.sz);
      break;
    }
    case OpJump: {
      i = op.data.sz - 1;
      break;
    }
    case OpJumpTrue:
    case OpJumpTruePop: {
      assert(!vms->empty());
      VarBase *var = vms->back();
      bool res = false;
      if (!var->toBool(vm, res, op.srcId, op.idx)) {
        vm.fail(op.srcId, op.idx, "cannot convert %s to bool",
                vm.getTypeName(var).c_str());
        vms->pop();
        execFail("cannot convert %s to bool", vm.getTypeName(var).c_str());
      }
      if (res)
        i = op.data.sz - 1;
      if (!res || op.op == OpJumpTruePop)
        vms->pop();
      break;
    }
    case OpJumpFalse:
    case OpJumpFalsePop: {
      assert(!vms->empty());
      VarBase *var = vms->back();
      bool res = false;
      if (!var->toBool(vm, res, op.srcId, op.idx)) {
        vm.fail(op.srcId, op.idx, "cannot convert %s to bool",
                vm.getTypeName(var).c_str());
        vms->pop();
        execFail("cannot convert %s to bool", vm.getTypeName(var).c_str());
      }
      if (!res)
        i = op.data.sz - 1;
      if (!res || op.op == OpJumpFalsePop)
        vms->pop();
      break;
    }
    case OpJumpNil: {
      if (vms->back()->isa<VarNil>()) {
        vms->pop();
        i = op.data.sz - 1;
      }
      break;
    }
    case OpBodyMarker: {
      bodies.push_back({i + 1, op.data.sz});
      i = op.data.sz - 1;
      break;
    }
    case OpMakeFunc: {
      std::string varArg;
      std::vector<std::string> args;
      if (op.data.s[0] == '1') {
        varArg = vms->back()->as<VarString>()->get();
        vms->pop();
      }

      size_t argSz = strlen(op.data.s);
      for (size_t i = 1; i < argSz; i++) {
        std::string name = vms->back()->as<VarString>()->get();
        vms->pop();
        args.push_back(name);
      }

      FnBodySpan body = bodies.back();
      bodies.pop_back();

      vms->push(new VarFunc(srcFile->path(), varArg, args, FnBody{.june = body},
                          false, op.srcId, op.idx));
      break;
    }
    case OpMemberCall:
    case OpCall: {
      args.clear();
      size_t len = strlen(op.data.s);
      bool memCall = op.op == OpMemberCall;
      bool vaUnpack = op.data.s[0] == '1';
      for (size_t i = 1; i < len; i++) {
        args.push_back(vms->pop(false));
      }

      VarBase *ctxBase = nullptr;
      VarBase *fnBase = nullptr;
      VarBase *res = nullptr;
      std::string fnName;
      if (vaUnpack) {
        if (!args.back()->isa<VarVec>()) {
          vm.fail(args.back()->srcId(), args.back()->idx(),
                  "cannot unpack non-vector value");
          for (auto &arg : args)
            varDref(arg);
          if (!memCall)
            varDref(fnBase);
          execFail("cannot unpack non-vector value");
        }
        VarVec *vec = args.back()->as<VarVec>();
        args.pop_back();
        for (auto &e : vec->get()) {
          varIref(e);
          args.push_back(e);
        }
        varDref(vec);
      }

      if (memCall) {
        fnName = vms->back()->as<VarString>()->get();
        vms->pop();
        ctxBase = vms->pop(false);
        if (ctxBase->isAttrBased())
          fnBase = ctxBase->attrGet(fnName);
        if (fnBase == nullptr)
          fnBase = vm.getTypeFn(ctxBase, fnName);
      } else {
        fnBase = vms->pop(false);
      }

      if (!fnBase) {
        if (memCall)
          vm.fail(ctxBase->srcId(), ctxBase->idx(),
                  "cannot find member '%s' on '%s'", fnName.c_str(),
                  vm.getTypeName(ctxBase).c_str());
        else
          vm.fail(fnBase->srcId(), fnBase->idx(), "cannot find function '%s'",
                  fnBase->as<VarString>()->get().c_str());
        varDref(ctxBase);
        for (auto &arg : args)
          varDref(arg);
        if (!memCall)
          varDref(fnBase);
        execFail("cannot find function '%s'",
                 fnBase->as<VarString>()->get().c_str());
      }

      if (!fnBase->isCallable()) {
        vm.fail(op.srcId, op.idx, "'%s' is not a function or struct definition",
                vm.getTypeName(fnBase).c_str());
        varDref(ctxBase);
        for (auto &arg : args)
          varDref(arg);
        if (!memCall)
          varDref(fnBase);
        execFail("'%s' is not a function or struct definition",
                 vm.getTypeName(fnBase).c_str());
      }

      args.insert(args.begin(), ctxBase);
      res = fnBase->call(vm, args, op.srcId, op.idx);

      if (!res) {
        // prevent showing the failure if the exec stack is too full
        // or we'll get an enourmous stack trace
        if (!vm.execStackCountExceeded) {
          vm.fail(op.srcId, op.idx, "'%s' call failed, see above",
                  vm.getTypeName(fnBase).c_str());
        }
        varDref(ctxBase);
        for (auto &arg : args)
          varDref(arg);
        if (!memCall)
          varDref(fnBase);
        execFail("'%s' call failed, see above", vm.getTypeName(fnBase).c_str());
      }

      if (!res->isa<VarNil>()) {
        vms->push(res, false);
      }
      for (auto &arg : args)
        varDref(arg);
      if (!memCall)
        varDref(fnBase);
      if (vm.exitCalled) {
        assert(jumps.size() == 0);
        if (!customBytecode)
          vars->popFn();
        vm.execStackCount--;
        return vm.exitCode;
      }
      break;
    }
    case OpAttr: {
      const std::string attr = op.data.s;
      VarBase *ctxBase = vms->pop(false);
      VarBase *val = nullptr;
      if (ctxBase->isAttrBased())
        val = ctxBase->attrGet(attr);
      if (val == nullptr)
        val = vm.getTypeFn(ctxBase, attr);
      if (val == nullptr) {
        vm.fail(op.srcId, op.idx, "type '%s' does not have attribute '%s'",
                vm.getTypeName(ctxBase).c_str(), attr.c_str());
        varDref(ctxBase);
        execFail("type '%s' does not have attribute '%s'",
                 vm.getTypeName(ctxBase).c_str(), attr.c_str());
      }
      varDref(ctxBase);
      vms->push(val);
      break;
    }
    case OpReturn: {
      if (!op.data.b) {
        vms->push(vm.nil);
      }
      assert(jumps.size() == 0);
      if (!customBytecode)
        vars->popFn();
      vm.execStackCount--;
      return vm.exitCode;
    }
    case OpPushLoop: {
      vars->pushLoop();
      break;
    }
    case OpPopLoop: {
      vars->popLoop();
      break;
    }
    case OpContinue: {
      vars->loopContinue();
      i = op.data.sz - 1;
      break;
    }
    case OpBreak: {
      i = op.data.sz - 1;
      break;
    }
    case OpPushJump: {
      jumps.push_back({nullptr, op.data.sz});
      vm.fails.blka();
      break;
    }
    case OpPushJumpNamed: {
      jumps.back().name = op.data.s;
      break;
    }
    case OpPopJump: {
      jumps.pop_back();
      vm.fails.blkr();
      break;
    }
    case _OpLast: {
      assert(false);
      break;
    }
    }
  }

  assert(jumps.size() == 0);
  if (!customBytecode)
    vars->popFn();
  vm.execStackCount--;
  return vm.exitCode;
}

} // namespace vm

} // namespace june
