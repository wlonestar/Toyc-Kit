//! interpreter

#ifndef INTERPRETER_H
#define INTERPRETER_H

#pragma once

#include <CodeGen/InterpreterCodeGen.h>
#include <Parser/InterpreterParser.h>

#include <string>

namespace toyc {

/**
 * @brief Toyc Interpreter class
 *
 */
class Interpreter {
private:
  InterpreterParser parser;
  InterpreterIRVisitor visitor;

private:
  /**
   * @brief For each type, use different function to handle
   *
   * @param unit - a variant of Expr, Stmt and Decl
   */
  void execute(InterpreterParser::parse_t &unit);

public:
  Interpreter() : parser(), visitor() {}

public:
  /**
   * @brief Parse `input` and execute using JIT
   *
   * @param input - input toyc script
   */
  void parseAndExecute(std::string input);
};

} // namespace toyc

#endif
