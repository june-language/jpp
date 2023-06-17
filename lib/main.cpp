#include "Common.hpp"
#include "VM/OpCodes.hpp"
#include <cxxopts.hpp>
#include <iostream>

auto main(i32 argc, i8 **argv) -> i32 {
  auto opts = cxxopts::Options("june", "The June VM bytecode interface");
  /* clang-format off */
  opts.add_options()
    ("d,debug", "Enable debugging")
    ("x,no-load", "Prevent loading specified Standard libraries",
      cxxopts::value<std::vector<std::string>>()->default_value("")->implicit_value(""))
    ("h,help", "Print this help message")
    ("v,version", "Print the version number")
    ;
  /* clang-format on */
  opts.allow_unrecognised_options();
  auto results = opts.parse(argc, argv);

  JuneDebug = results.count("debug") || JuneDebug;

  DebugLog << "Debugging enabled" << std::endl;

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

  june::vm::Bytecode bc;
  bc.addNil(0, june::vm::Ops::Return);

  june::fs::writeBytes("test.jbc", bc.toBytes());

  std::cout << "Done" << std::endl;

  return 0;
}
