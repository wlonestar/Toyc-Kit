//! interpreter

#ifndef INTERPRETER_H
#define INTERPRETER_H

#pragma once

#include <CodeGen.h>
#include <Parser.h>

#include <string>

namespace toyc {

class Interpreter {
private:
  Parser parser;

private:
  void writeByteCode(std::string &filename);

public:
  Interpreter() : parser() { initializeModule(); }

  void compile(std::string &input);
};

} // namespace toyc

#endif
