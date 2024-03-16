//! parser of toyc

#ifndef INTERPRETER_PARSER_H
#define INTERPRETER_PARSER_H

#pragma once

#include <Parser/Parser.h>

#include <memory>
#include <variant>

namespace toyc {

/**
 * @brief Inherited from `BaseParser`
 *
 */
class InterpreterParser : public BaseParser {
private:
  using expr_or_stmt_t = std::pair<std::unique_ptr<ExprStmt>, bool>;

public:
  using parse_t = std::variant<std::unique_ptr<Decl>, std::unique_ptr<Stmt>,
                               std::unique_ptr<Expr>>;

private:
  expr_or_stmt_t parseExprOrExprStmt();

  virtual std::unique_ptr<Decl> parseVariableDeclaration(std::string &type,
                                                 std::string &name,
                                                 VarScope scope) override;

public:
  InterpreterParser() : BaseParser() {}

public:
  /**
   * @brief Interpreter parse entry, support expressions, statements and
   * declarations
   *
   * @notice: Variable declarations are all global variable
   *
   * @return parse_t - A variant of three types
   */
  parse_t parse();
};

} // namespace toyc

#endif
