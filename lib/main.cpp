#include "Common.hpp"
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

  return 0;
}
