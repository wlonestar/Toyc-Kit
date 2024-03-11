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

public:
  Compiler() : preprocessor(), parser(), visitor() {}

  /**
   * @brief compile source code to byte code (IR)
   *
   * @param src source code filepath
   * @param os output stream
   */
  void compile(std::string &src, llvm::raw_ostream &os);
};

} // namespace toyc

#endif
