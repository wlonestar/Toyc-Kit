//! Parser implementation

#include <AST.h>
#include <CodeGen.h>
#include <Parser.h>
#include <Token.h>
#include <Util.h>

#include <llvm-16/llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace toyc {

std::map<std::string, std::pair<std::string, llvm::Value *>> VariableTable;

std::map<std::string,
         std::tuple<std::string, llvm::AllocaInst *, llvm::Value *>>
    LocalVariableTable;

void clearLocalVarTable() { LocalVariableTable.clear(); }

void printVariableTable() {
  std::cout << fstr("\033[1;32mVariableTable({}):\033[0m\n",
                    VariableTable.size());
  for (auto &[first, val] : VariableTable) {
    std::string value_str;
    llvm::raw_string_ostream ros(value_str);
    if (val.second != nullptr) {
      val.second->print(ros);
    } else {
      ros << "null";
    }
    std::cout << fstr("\033[1;32m  <var> '{}': {}\033[0m\n", first, value_str);
  }
}

void printLocalVariableTable() {
  std::cout << fstr("\033[1;32mLocalVariableTable({}):\033[0m\n",
                    VariableTable.size());
  for (auto &[first, val] : VariableTable) {
    std::string value_str;
    llvm::raw_string_ostream ros(value_str);
    if (val.second != nullptr) {
      val.second->print(ros);
    } else {
      ros << "null";
    }
    std::cout << fstr("\033[1;32m  <var> '{}': {}\033[0m\n", first, value_str);
  }
}

int64_t Parser::parseIntegerSuffix(std::string &value, int base) {
  try {
    if (value.ends_with("llu") || value.ends_with("llU") ||
        value.ends_with("LLu") || value.ends_with("LLU") ||
        value.ends_with("ull") || value.ends_with("uLL") ||
        value.ends_with("Ull") || value.ends_with("ULL")) {
      return stoull(value, nullptr, base);
    } else if (value.ends_with("ul") || value.ends_with("uL") ||
               value.ends_with("Ul") || value.ends_with("UL") ||
               value.ends_with("lu") || value.ends_with("lU") ||
               value.ends_with("Lu") || value.ends_with("LU")) {
      return stoul(value, nullptr, base);
    } else if (value.ends_with("ll") || value.ends_with("LL")) {
      return stoll(value, nullptr, base);
    } else if (value.ends_with("l") || value.ends_with("L")) {
      return stol(value, nullptr, base);
    } else if (value.ends_with("u") || value.ends_with("U")) {
      return static_cast<unsigned int>(stoul(value, nullptr, base));
    } else {
      return stoi(value, nullptr, base);
    }
  } catch (std::out_of_range e) {
    throwParserError(
        "integer literal is too large to be represented in integer type");
    return -1;
  }
}

double Parser::parseFloatingSuffix(std::string &value, int base) {
  try {
    if (value.ends_with("f") || value.ends_with("F")) {
      return std::stof(value, nullptr);
    } else if (value.ends_with("l") || value.ends_with("L")) {
      return stold(value, nullptr);
    } else {
      return stod(value, nullptr);
    }
  } catch (std::out_of_range e) {
    throwParserError("magnitude of floating-point constant too large");
    return -0;
  }
}

/**
 * parse Expr
 */

std::unique_ptr<Expr> Parser::parseIntegerLiteral() {
  auto value_str = previous().value;
  int64_t value;
  if (checkHexadecimal(value_str)) {
    value_str.erase(0, 2);
    value = parseIntegerSuffix(value_str, 16);
  } else if (checkOctal(value_str)) {
    value_str.erase(0, 1);
    value = parseIntegerSuffix(value_str, 8);
  } else if (value_str.starts_with("'")) {
    /// TODO: not support escape sequence yet
    value_str.erase(0, 1); // remove leading '
    value_str.pop_back();  // remove suffix '
    int value = value_str[0];
    return std::make_unique<CharacterLiteral>(value);
  } else {
    value = parseIntegerSuffix(value_str, 10);
  }
  return std::make_unique<IntegerLiteral>(value, "i64");
}

std::unique_ptr<Expr> Parser::parseFloatingLiteral() {
  auto value_str = previous().value;
  auto value = parseFloatingSuffix(value_str, 10);
  return std::make_unique<FloatingLiteral>(value, "f64");
}

std::unique_ptr<Expr> Parser::parsePrimaryExpression() {
  if (match(INTEGER)) {
    return parseIntegerLiteral();
  }
  if (match(FLOATING)) {
    return parseFloatingLiteral();
  }
  if (match(STRING)) {
    auto value = previous().value;
    /// add a terminator '\0' size
    auto type = fstr("char[{}]", value.size() + 1);
    return std::make_unique<StringLiteral>(std::move(value), std::move(type));
  }
  if (match(IDENTIFIER)) {
    Token token = previous();
    std::string name = token.value;
    std::string type = std::get<0>(LocalVariableTable[name]);
    return std::make_unique<DeclRefExpr>(type, name);
  }
  if (match(LP)) {
    auto expr = parseExpression();
    consume(RP, "expected ')'");
    expr = std::make_unique<ParenExpr>(std::move(expr));
    return expr;
  }
  throwParserError("parse primary expression error");
  return nullptr;
}

std::unique_ptr<Expr> Parser::parseUnaryExpression() {
  if (match({ADD, NOT, SUB})) {
    auto op = previous();
    auto right = parseUnaryExpression();
    auto type = checkUnaryOperatorType(op.type, right.get());
    return std::make_unique<UnaryOperator>(op, std::move(right),
                                           std::move(type));
  }
  return parsePrimaryExpression();
}

std::unique_ptr<Expr> Parser::parseMultiplicativeExpression() {
  auto expr = parseUnaryExpression();
  while (match({MUL, DIV, MOD})) {
    auto op = previous();
    auto right = parseUnaryExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseAdditiveExpression() {
  auto expr = parseMultiplicativeExpression();
  while (match({ADD, SUB})) {
    auto op = previous();
    auto right = parseMultiplicativeExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseRelationalExpression() {
  auto expr = parseAdditiveExpression();
  while (match({LE_OP, GE_OP, LA, RA})) {
    auto op = previous();
    auto right = parseAdditiveExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseEqualityExpression() {
  auto expr = parseRelationalExpression();
  while (match({EQ_OP, NE_OP})) {
    auto op = previous();
    auto right = parseRelationalExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalAndExpression() {
  auto expr = parseEqualityExpression();
  while (match(AND_OP)) {
    auto op = previous();
    auto right = parseEqualityExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalOrExpression() {
  auto expr = parseLogicalAndExpression();
  while (match(OR_OP)) {
    auto op = previous();
    auto right = parseLogicalAndExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseAssignmentExpression() {
  auto expr = parseLogicalOrExpression();
  if (match(EQUAL)) {
    auto token = previous();
    auto right = parseAssignmentExpression();
    return std::make_unique<BinaryOperator>(token, std::move(expr),
                                            std::move(right), right->getType());
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseExpression() {
  return parseAssignmentExpression();
}

/**
 * parse Stmt
 */

std::unique_ptr<Stmt> Parser::parseExpressionStatement() {
  auto expr = parseExpression();
  consume(SEMI, "expected ';' after expression");
  return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseReturnStatement() {
  if (match(SEMI)) {
    return std::make_unique<ReturnStmt>(nullptr);
  }
  auto expr = parseExpression();
  consume(SEMI, "expected ';' after expression");
  return std::make_unique<ReturnStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseDeclarationStatement() {
  auto type = previous().value;
  if (match(IDENTIFIER)) {
    auto name = previous().value;
    auto decl =
        parseVariableDeclaration(std::move(type), std::move(name), LOCAL);
    return std::make_unique<DeclStmt>(std::move(decl));
  }
  throwParserError("expected identifier");
  return nullptr;
}

std::unique_ptr<Stmt> Parser::parseCompoundStatement() {
  consume(LC, "expected function body after function declarator");
  std::vector<std::unique_ptr<Stmt>> stmts;
  while (!check(RC) && current.type != _EOF) {
    auto stmt = parseStatement();
    stmts.push_back(std::move(stmt));
  }
  consume(RC, "expected '}'");
  return std::make_unique<CompoundStmt>(std::move(stmts));
}

std::unique_ptr<Stmt> Parser::parseStatement() {
  if (match(RETURN)) {
    return parseReturnStatement();
  }
  if (match({VOID, I64, F64})) {
    return parseDeclarationStatement();
  }
  return parseExpressionStatement();
}

/**
 * internal parse
 */

std::string Parser::parseDeclarationSpecifiers() {
  if (match({VOID, I64, F64})) {
    return previous().value;
  }
  throwParserError("expected type specifier");
  return "";
}

std::string Parser::parseDeclarator() {
  if (match(IDENTIFIER)) {
    return previous().value;
  }
  throwParserError("expected identifier");
  return "";
}

/**
 * parse Decl
 */

std::unique_ptr<Decl> Parser::parseVariableDeclaration(std::string &&type,
                                                       std::string &&name,
                                                       VarScope scope) {
  if (scope == GLOBAL) {
    if (VariableTable.find(name) != VariableTable.end()) {
      throwParserError(fstr("redefinition of '{}'", name));
    }
    VariableTable[name].first = type;
  } else {
    if (LocalVariableTable.find(name) != LocalVariableTable.end()) {
      throwParserError(fstr("redefinition of '{}'", name));
    }
    std::get<0>(LocalVariableTable[name]) = type;
  }

  std::unique_ptr<Expr> init =
      (match(EQUAL) ? parseAssignmentExpression() : nullptr);
  std::unique_ptr<VarDecl> decl = std::make_unique<VarDecl>(
      std::move(type), std::move(name), std::move(init), scope);
  consume(SEMI, "expected ';' after declaration");
  return decl;
}

std::unique_ptr<Decl> Parser::parseFunctionDeclaration(std::string &&type,
                                                       std::string &&name) {
  consume(RP, "expected parameter declarator");
  /// only declaration
  if (match(SEMI)) {
    return std::make_unique<FunctionDecl>(std::move(name), fstr("{} ()", type),
                                          std::vector<std::unique_ptr<Decl>>{});
  }
  clearLocalVarTable();
  auto body = parseCompoundStatement();
  // printLocalVariableTable();
  return std::make_unique<FunctionDecl>(std::move(name), fstr("{} ()", type),
                                        std::vector<std::unique_ptr<Decl>>{},
                                        std::move(body));
}

std::unique_ptr<Decl> Parser::parseExternalDeclaration() {
  auto type = parseDeclarationSpecifiers();
  auto name = parseDeclarator();
  if (match(LP)) {
    return parseFunctionDeclaration(std::move(type), std::move(name));
  } else {
    return parseVariableDeclaration(std::move(type), std::move(name), GLOBAL);
  }
}

std::unique_ptr<TranslationUnitDecl> Parser::parse() {
  std::vector<std::unique_ptr<Decl>> decls;
  advance();
  while (current.type != _EOF) {
    auto decl = parseExternalDeclaration();
    decls.push_back(std::move(decl));
  }
  return std::make_unique<TranslationUnitDecl>(std::move(decls));
}

/**
 * Constructor of Parser Error object
 */

ParserError::ParserError(size_t _line, size_t _col, std::string &_message)
    : line(_line), col(_col),
      message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                   "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                   _line, _col, _message)) {}

ParserError::ParserError(size_t _line, size_t _col, std::string &&_message)
    : line(_line), col(_col),
      message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                   "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                   _line, _col, _message)) {}

} // namespace toyc
