//! entry point of toyc

#include <Interpreter/Interpreter.h>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

using namespace toyc;
using namespace std;

int main(int argc, const char **argv) {
  if (argc < 2) {
    std::cerr << fstr("Usage: {} <src>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  Interpreter interpreter;
  std::string input;
  read_from(argv[1], input);
  interpreter.compile(input);
  return 0;
}
