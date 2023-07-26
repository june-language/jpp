#include "Common.hpp"
#include "VM/OpCodes.hpp"
#include <cxxopts.hpp>
#include <iostream>

auto main(i32 argc, i8 **argv) -> i32 {
  auto opts = cxxopts::Options("june", "The June VM bytecode interface");
  /* clang-format off */
  opts.add_options("VM Operations")
    ("x,no-load", "Prevent loading specified Standard libraries",
      cxxopts::value<std::vector<std::string>>()->default_value("")->implicit_value(""))
  ;
  opts.add_options("VM Debugging")
    ("d,debug", "Enable debugging (memory debugging not enabled)")
    ("w,write-test", "Write a test bytecode file to the current directory (ignores `input`)")
  ;
  opts.add_options("General")
    ("h,help", "Print this help message")
    ("v,version", "Print the version number")
    ("input", "Input bytecode file", cxxopts::value<std::string>())
    ;
  /* clang-format on */
  opts.allow_unrecognised_options();
  opts.parse_positional({"input"});

  cxxopts::ParseResult results;
  try {
    results = opts.parse(argc, argv);
  } catch (const cxxopts::exceptions::exception &e) {
    std::cerr << "Error parsing options: " << e.what() << std::endl;
    return 1;
  }

  JuneDebug = results.count("debug") || JuneDebug;

  if (results.count("help")) {
    std::cout << opts.help() << std::endl;
    return 0;
  }

  if (results.count("version")) {
    std::cout << "June " << JuneVersion << " (" << JuneGitRev << ", "
              << JuneBuildDate << ")" << std::endl;
    if (JuneDebug || JuneMemDebug) {
      if (JuneMemDebug) {
        std::cout << "Debug build, memory debugging enabled";
      } else {
        std::cout << "Debug mode, memory debugging disabled";
      }

      std::cout << std::endl;
    }

    return 0;
  }

  DebugLog << "Debugging enabled" << std::endl;

  if (results.count("write-test")) {
    june::vm::Bytecode bc;
    bc.addStr(0, june::vm::Ops::Load, june::vm::OpDataType::Ident, "print");
    bc.addStr(1, june::vm::Ops::Load, june::vm::OpDataType::String, "Hello, world!");
    bc.addStr(2, june::vm::Ops::Call, june::vm::OpDataType::String, "00");
    bc.addNil(3, june::vm::Ops::Pop);

    june::fs::writeBytes("test.jbc", bc.toBytes()).expect("failed to write file");
    std::cout << "Successful write. Bytecode size: " << bc.size() << " ops" << std::endl;
  } else {
    auto res = june::fs::readFileBytes(results["input"].as<std::string>());
    if (!res) {
      std::cerr << "Failed to read file: " << res.error() << std::endl;
      return 1;
    }

    auto bytes = res.value();
    auto bcRes = june::vm::Bytecode::fromBytes(bytes);
    if (!bcRes) {
      std::cerr << "Failed to parse bytecode: " << bcRes.error() << std::endl;
      return 1;
    }

    auto bc = bcRes.value();
    std::cout << "Successful read. Bytecode size: " << bc.size() << " ops" << std::endl;
  }

  return 0;
}
