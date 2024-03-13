//! parser of toyc

#ifndef INTERPRETER_PARSER_H
#define INTERPRETER_PARSER_H

#pragma once

#include <AST/AST.h>
#include <Lexer/Lexer.h>
#include <Parser/Parser.h>
#include <Util.h>

#include <cstddef>
#include <exception>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace toyc {

class InterpreterParser : public BaseParser {
private:
  using parse_t = std::variant<std::unique_ptr<Decl>, std::unique_ptr<Stmt>,
                               std::unique_ptr<Expr>>;
  using expr_or_stmt_t = std::pair<std::unique_ptr<ExprStmt>, bool>;

  expr_or_stmt_t parseExprOrExprStmt();

public:
  InterpreterParser() : BaseParser() {}

  parse_t parse();
};

} // namespace toyc

#endif
