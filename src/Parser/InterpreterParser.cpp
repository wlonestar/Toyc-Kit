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

auto InterpreterParser::ParseExprOrExprStmt() -> InterpreterParser::ExprOrStmt {
  ExprPtr expr = nullptr;
  bool is_stmt = true;
  if (!Match(SEMI)) {
    expr = ParseExpression();
    if (!Match(SEMI)) {
      is_stmt = false;
    }
  }
  return {std::make_unique<ExprStmt>(std::move(expr)), is_stmt};
}

auto InterpreterParser::ParseVariableDeclaration(std::string type,
                                                 std::string name,
                                                 VarScope scope) -> DeclPtr {
  ExprPtr init;
  if (scope == GLOBAL) {
    if (global_var_table_.find(name) != global_var_table_.end()) {
      throw ParserException(loc_, makeString("redefinition of '{}'", name));
    }
    init = (Match(EQUAL) ? ParseAssignmentExpression() : nullptr);
    global_var_table_[name] = type;
  } else {
    if (var_table_.find(name) != var_table_.end()) {
      throw ParserException(loc_, makeString("redefinition of '{}'", name));
    }
    var_table_[name] = type;
    init = (Match(EQUAL) ? ParseAssignmentExpression() : nullptr);
  }

  if (init != nullptr && type != init->GetType()) {
    init = std::make_unique<ImplicitCastExpr>(type, std::move(init));
  }
  std::unique_ptr<VarDecl> decl = std::make_unique<VarDecl>(
      std::move(name), std::move(type), std::move(init), scope);
  Consume(SEMI, "expected ';' after declaration");

  return decl;
}

auto InterpreterParser::Parse() -> InterpreterParser::ParseResult {
  std::string type;
  std::string name;
  bool is_extern = false;
  if (Match(EXTERN)) {
    is_extern = true;
  }
  /// declaration
  if (Match({VOID, I64, F64})) {
    type = Previous().value_;
    if (Match(IDENTIFIER)) {
      name = Previous().value_;
    } else {
      throw ParserException(loc_, "invalid expression");
    }
    if (Match(LP)) {
      return ParseFunctionDeclaration(type, name, is_extern);
    }
    return ParseVariableDeclaration(type, name, GLOBAL);
  }
  if (Check({IF, WHILE, FOR, LC})) {
    return ParseStatement();
  }
  auto [stmt, flag] = ParseExprOrExprStmt();
  if (flag) {
    return std::move(stmt);
  }
  return std::move(stmt->expr_);
}

} // namespace toyc
