//! InterpreterParser implementation

#include <Parser/InterpreterParser.h>

#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace toyc {

InterpreterParser::expr_or_stmt_t InterpreterParser::parseExprOrExprStmt() {
  std::unique_ptr<Expr> expr = nullptr;
  bool isStmt = true;
  if (!match(SEMI)) {
    expr = parseExpression();
    if (!match(SEMI)) {
      isStmt = false;
    }
  }
  return {std::make_unique<ExprStmt>(std::move(expr)), isStmt};
}

InterpreterParser::parse_t InterpreterParser::parse() {
  parse_t translationUnit;

  advance();
  std::string type, name;
  bool isExtern = false;
  if (match(EXTERN)) {
    isExtern = true;
  }
  /// declaration
  if (match({VOID, I64, F64})) {
    std::cout << fstr("parse declaration\n");
    type = previous().value;
    if (match(IDENTIFIER)) {
      name = previous().value;
    } else {
      throwParserException("invalid expression");
    }
    if (match(LP)) {
      return parseFunctionDeclaration(type, name, isExtern);
    } else {
      return parseVariableDeclaration(type, name, GLOBAL);
    }
  } else if (check({IF, WHILE, FOR, LC})) {
    std::cout << fstr("parse statement\n");
    return parseStatement();
  } else {
    auto [stmt, flag] = parseExprOrExprStmt();
    if (flag) {
      std::cout << fstr("parse statement\n");
      return std::move(stmt);
    } else {
      std::cout << fstr("parse expression\n");
      return std::move(stmt->expr);
    }
  }
}

} // namespace toyc
