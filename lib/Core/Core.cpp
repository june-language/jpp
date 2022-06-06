#include <VM/State.hpp>
#include <cstdio>

using namespace june;

VarBase *print(State &vm, const FnData &fd) {
  for (size_t i = 1; i < fd.args.size(); ++i) {
    std::string str;
    if (!fd.args[i]->toStr(vm, str, fd.srcId, fd.idx)) {
      return nullptr;
    }
    printf("%s", str.c_str());
  }
  printf("\n");

  return vm.nil;
}

VarBase *import(State &vm, const FnData &fd) {
  VarBase *entry = fd.args[1];
  if (!entry->isa<VarString>()) {
    vm.fail(entry->srcId(), entry->idx(),
            "expected argument to be of type string, found: %s",
            vm.getTypeName(entry->type()).c_str());
    return nullptr;
  }
  VarString *importFile = entry->as<VarString>();

  auto err = vm.juneModuleLoad(importFile->get(), fd.srcId, fd.idx);
  if (err.isErr()) {
    vm.fail(fd.srcId, fd.idx, "failed to import module '%s': %s",
            importFile->get().c_str(), err.unwrapErr().toString().c_str());
    return nullptr;
  }

  return vm.allSrcs[importFile->get()];
}

VarBase *importNative(State &vm, const FnData &fd) {
  VarBase *entry = fd.args[1];
  if (!entry->isa<VarString>()) {
    vm.fail(entry->srcId(), entry->idx(),
            "expected argument to be of type string, found: %s",
            vm.getTypeName(entry->type()).c_str());
    return nullptr;
  }
  VarString *importFile = entry->as<VarString>();

  if (!vm.nativeModuleLoad(importFile->get(), fd.srcId, fd.idx)) {
    vm.fail(fd.srcId, fd.idx, "failed to load native module '%s'",
            importFile->get().c_str());
    return nullptr;
  }

  return vm.nil;
}

extern "C" bool june_init(State &vm, const size_t srcId, const size_t &idx) {
  const auto &srcName = vm.currentSourceFile()->path();

  vm.globalAdd("print", new VarFunc(srcName, ".", {}, {.native = print}, true,
                                    srcId, idx));
  vm.globalAdd("import", new VarFunc(srcName, "", {""}, {.native = import},
                                     true, srcId, idx));
  vm.globalAdd("importNative", new VarFunc(srcName, "", {""},
                                                {.native = importNative},
                                                true, srcId, idx));

  return true;
}
