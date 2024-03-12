//! interpreter

#ifndef INTERPRETER_H
#define INTERPRETER_H

#pragma once

#include <CodeGen/InterpreterCodeGen.h>
#include <Parser/InterpreterParser.h>

#include <string>

namespace toyc {

class Interpreter {
private:
  InterpreterParser parser;
  InterpreterIRCodegenVisitor visitor;

public:
  Interpreter() : parser(), visitor() {}

  void compile(std::string &input);
};

} // namespace toyc

#endif
