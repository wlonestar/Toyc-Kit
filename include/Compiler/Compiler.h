//! toyc compiler class

#ifndef COMPILER_H
#define COMPILER_H

#pragma once

#include <CodeGen/CodeGen.h>
#include <Parser/Parser.h>
#include <Preprocessor/Preprocessor.h>

namespace toyc {

/**
 * @brief compiler frontend
 *
 * source code -> byte code
 */
class Compiler {
private:
  Preprocessor preprocessor;
  Parser parser;
  IRCodegenVisitor visitor;

private:
  bool readFrom(std::string &src, std::string &input);
  bool writeTo(std::string &dest);

public:
  Compiler(char *arg0) : preprocessor(arg0), parser(), visitor() {}

  /**
   * @brief compile source code to byte code (IR)
   *
   * @param src source code filepath
   * @param dest bytecode filepath
   */
  void compile(std::string &src, std::string &dest);
};

} // namespace toyc

#endif
