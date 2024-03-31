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
  using ExprOrStmt = std::pair<std::unique_ptr<ExprStmt>, bool>;

public:
  using ParseResult = std::variant<DeclPtr, StmtPtr, ExprPtr>;

private:
  auto ParseExprOrExprStmt() -> ExprOrStmt;

  auto ParseVariableDeclaration(std::string type, std::string name,
                                VarScope scope) -> DeclPtr override;

public:
  InterpreterParser() = default;

public:
  /**
   * @brief Interpreter parse entry, support expressions, statements and
   * declarations
   *
   * @notice: Variable declarations are all global variable
   *
   * @return parse_t - A variant of three types
   */
  auto Parse() -> ParseResult;
};

} // namespace toyc

#endif
