#include "Common.hpp"
#include "JuneConfig.hpp"
#include "VM/State.hpp"
#include <cctype>
#include <iostream>
#include <vector>

using namespace june;
using namespace june::args;
using namespace june::err;
using namespace june::fs;

err::Errors JuneReadCode(const SrcFile *src, const std::string &srcDir,
                         const std::string &srcPath, Bytecode &bc,
                         const bool isMainSrc, const bool exprOnly,
                         const size_t &beginIdx, const size_t &endIdx) {
  bc.adds(0, OpLoad, OdtIdent, "print");
  bc.adds(1, OpLoad, OdtString, "Hello, World!");
  bc.adds(2, OpCall, OdtString, "00");
  bc.add(3, OpUnload);

  // if it's a bytecode file, it'll already be loaded
  // if (!src->isBytecode()) {
  //   return err::Errors::Err(
  //       Error(err::ErrFileIo, "file reading unimplemented"));
  // }

  return err::Errors::Ok();
}

SrcFile *JuneLoadCode(const std::string &srcFile, const std::string &srcDir,
                      const bool isMainSrc, err::Errors &err,
                      const size_t &beginIdx, const size_t &endIdx) {
  auto src = new SrcFile(srcDir, srcFile, isMainSrc);
  auto srcRes = src->loadFile();
  if (srcRes.isErr()) {
    srcRes.getErr()->print(std::cerr);
    delete src;
    return nullptr;
  }

  auto loadRes = JuneReadCode(src, src->dir(), src->path(), src->bytecode(),
                              isMainSrc, false, beginIdx, endIdx);
  if (loadRes.isErr()) {
    loadRes.getErr()->print(std::cerr);
    delete src;
    return nullptr;
  }

  for (auto &bc : src->bytecode().getMut()) {
    bc.srcId = src->id();
  }

  return src;
}

int main(int argc, char **argv) {
  ArgsAddArgument("help", "-h", "--help", "Print this help message");
  ArgsAddArgument("version", "-v", "--version", "Print the version");
  ArgsParseArguments(argc, argv);

  if (!ArgsAnyArgumentExists()) {
    ArgsPrintHelp(argv[0]);
    return 0;
  }

  if (ArgsArgumentExists("help")) {
    ArgsPrintHelp(argv[0]);
  } else if (ArgsArgumentExists("version")) {
    std::cerr << "Running on June v" << JuneVersion << std::endl;
    std::cerr << "On Git Revision: " << JuneGitRev << std::endl;
    std::cerr << "Built on: " << JuneBuildDate << std::endl;
    return 0;
  }

  std::string juneBase, juneBin;
  juneBin = fs::absPath(env::getProcPath(), &juneBase, true);
  State vm(juneBin, juneBase, ArgsGetCodeArgs());

  auto mainFileArg = ArgsGetPositional(0);
  if (!fs::exists(mainFileArg.value).unwrap()) {
    std::cerr << "File not found: " << mainFileArg.value << std::endl;
    return 1;
  }

  std::string mainDir;
  std::string mainFile = fs::absPath(mainFileArg.value, &mainDir);

  err::Errors err = Errors::Ok();
  auto mainSrc = JuneLoadCode(mainFile, mainDir, true, err, 0, 0);

  if (err.isErr()) {
    err.getErr()->print(std::cerr);
    std::cerr << "Failed to load main file: " << err.getErr()->toString()
              << std::endl;
    return 1;
  }

  vm.pushSrc(mainSrc, 0);
  if (!vm.loadCoreModules()) {
    vm.popSrc();
    err.getErr()->print(std::cerr);
    std::cerr << "Failed to load core modules" << std::endl;
    return 1;
  }

  auto execErr = vm::exec(vm);
  vm.popSrc();
  if (execErr.isErr()) {
    execErr.getErr()->print(std::cerr);
    std::cerr << "Failed to execute main file" << std::endl;
    return 1;
  }

  return execErr.unwrap();
}
