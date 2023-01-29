#include "Common.hpp"
#include "VM/Memory.hpp"
#include <cxxopts.hpp>
#include <iostream>

auto main(int argc, char **argv) -> int {
  auto opts = cxxopts::Options("june", "The June VM bytecode interface");
  /* clang-format off */
  opts.add_options()
    ("d,debug", "Enable debugging")
    ("x,no-load", "Prevent loading specified Standard libraries",
      cxxopts::value<std::vector<std::string>>())
    ;
  /* clang-format on */
  opts.allow_unrecognised_options();
  auto results = opts.parse(argc, argv);

  JuneDebug = results.count("debug") || JuneDebug;

  DebugLog << "Debugging enabled" << std::endl;

  auto kb = 1024;
  auto mb = kb * 1024;
  auto gb = mb * 1024;

  auto ptr = june::mem::alloc(1.5 * gb);
  june::mem::zero(ptr, 1.5 * gb);

  std::cin.get();

  june::mem::free(ptr, 1.5 * gb);

  return 0;
}
