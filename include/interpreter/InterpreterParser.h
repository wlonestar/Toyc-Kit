//! parser of toyc

#ifndef INTERPRETER_PARSER_H
#define INTERPRETER_PARSER_H

#pragma once

#include <AST.h>
#include <Lexer.h>
#include <Token.h>
#include <Util.h>
#include <compiler/CompilerParser.h>

#include <cstddef>
#include <exception>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace toyc {

class InterpreterParser : public CompilerParser {
public:
  InterpreterParser() : CompilerParser() {}

  virtual std::unique_ptr<Decl>
  parseVariableDeclaration(std::string &type, std::string &name,
                           VarScope scope = LOCAL) override;

  virtual std::unique_ptr<Decl> parseExternalDeclaration() override;

  virtual std::unique_ptr<TranslationUnitDecl> parse() override;
};

} // namespace toyc

#endif
