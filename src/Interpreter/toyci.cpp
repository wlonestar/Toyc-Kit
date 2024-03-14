//! entry point of toyc

#include <Config.h>
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
  std::string src(argv[1]);
  if (!src.ends_with(toyc::script_ext)) {
    std::cerr << fstr("incorrect file extension\n");
    exit(EXIT_FAILURE);
  }

  read_from(src, input);
  interpreter.compile(input);
  return 0;
}
