//! interpreter

#ifndef INTERPRETER_H
#define INTERPRETER_H

#pragma once

#include <interpreter/InterpreterCodeGen.h>
#include <interpreter/InterpreterParser.h>

#include <string>

namespace toyc {

class Interpreter {
private:
  InterpreterParser parser;
  InterpreterCodegenVisitor visitor;

public:
  Interpreter() : parser(), visitor() {}

  void compile(std::string &input);
};

} // namespace toyc

#endif
