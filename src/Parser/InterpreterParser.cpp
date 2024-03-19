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

InterpreterParser::ExprOrStmt InterpreterParser::parseExprOrExprStmt() {
  ExprPtr expr = nullptr;
  bool isStmt = true;
  if (!match(SEMI)) {
    expr = parseExpression();
    if (!match(SEMI)) {
      isStmt = false;
    }
  }
  return {std::make_unique<ExprStmt>(std::move(expr)), isStmt};
}

DeclPtr InterpreterParser::parseVariableDeclaration(std::string type,
                                                    std::string name,
                                                    VarScope scope) {
  ExprPtr init;
  if (scope == GLOBAL) {
    if (globalVarTable.find(name) != globalVarTable.end()) {
      throwParserException(fstr("redefinition of '{}'", name));
    }
    init = (match(EQUAL) ? parseAssignmentExpression() : nullptr);
    globalVarTable[name] = type;
  } else {
    if (varTable.find(name) != varTable.end()) {
      throwParserException(fstr("redefinition of '{}'", name));
    }
    varTable[name] = type;
    init = (match(EQUAL) ? parseAssignmentExpression() : nullptr);
  }

  if (init != nullptr && type != init->getType()) {
    init = std::make_unique<ImplicitCastExpr>(type, std::move(init));
  }
  std::unique_ptr<VarDecl> decl = std::make_unique<VarDecl>(
      std::move(name), std::move(type), std::move(init), scope);
  consume(SEMI, "expected ';' after declaration");

  return decl;
}

InterpreterParser::ParseResult InterpreterParser::parse() {
  std::string type, name;
  bool isExtern = false;
  if (match(EXTERN)) {
    isExtern = true;
  }
  /// declaration
  if (match({VOID, I64, F64})) {
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
    return parseStatement();
  } else {
    auto [stmt, flag] = parseExprOrExprStmt();
    if (flag) {
      return std::move(stmt);
    } else {
      return std::move(stmt->expr);
    }
  }
}

} // namespace toyc
