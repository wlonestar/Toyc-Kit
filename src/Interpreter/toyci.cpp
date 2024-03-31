//! entry point of toyc

#include <Config.h>
#include <Interpreter/Interpreter.h>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

auto main(int argc, const char **argv) -> int {
  if (argc < 2) {
    std::cerr << makeString("Usage: {} <src>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  toyc::Interpreter interpreter;
  std::string input;
  std::string src(argv[1]);
  if (!src.ends_with(toyc::script_ext)) {
    std::cerr << makeString("incorrect file extension\n");
    exit(EXIT_FAILURE);
  }

  toyc::ReadFrom(src, input);
  interpreter.ParseAndExecute(input);
  return 0;
}
