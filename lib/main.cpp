#include "JuneConfig.hpp"
#include <iostream>
#include <ostream>

/*
 * Currently, just boilerplate.
 * Planning for the c++ rewrite
 * is going to take a while...
 */

int main() {
  std::cout << "Running June on version: v" << JuneVersion << std::endl;
  std::cout << "On Git Revision: " << JuneGitRev << std::endl;
  std::cout << "Built on: " << JuneBuildDate << std::endl;

  return 0;
}

