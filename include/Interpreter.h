// ! interpreter

#ifndef INTERPRETER_H
#define INTERPRETER_H

#pragma once

#include <string>

namespace toyc {

class Interpreter {
public:
  Interpreter() = default;

  void compile(std::string &input);
};

} // namespace toyc

#endif
