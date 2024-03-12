//! InterpreterParser implementation

#include "Lexer/Token.h"
#include <AST/AST.h>
#include <Parser/InterpreterParser.h>
#include <Util.h>

#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace toyc {

Token InterpreterParser::advance() {
  prev = current;
  for (;;) {
    current = lexer.scanToken();
    if (current.type != ERROR) {
      break;
    }
    throwInterParserException("error at parse");
  }
  // debug("prev:{}, curent:{}", prev.toString(), current.toString());
  return prev;
}

Token InterpreterParser::consume(TokenType type, std::string &message) {
  if (current.type == type) {
    return advance();
  }
  throwInterParserException(message);
  return Token(ERROR, "");
}

Token InterpreterParser::consume(TokenType type, std::string &&message) {
  if (current.type == type) {
    return advance();
  }
  throwInterParserException(message);
  return Token(ERROR, "");
}

bool InterpreterParser::check(std::initializer_list<TokenType> types) {
  for (TokenType type : types) {
    if (type == _EOF) {
      return false;
    }
    if (peek().type == type) {
      return true;
    }
  }
  return false;
}

bool InterpreterParser::check(TokenType type) {
  if (type == _EOF) {
    return false;
  }
  return peek().type == type;
}

bool InterpreterParser::match(std::initializer_list<TokenType> types) {
  for (TokenType type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

bool InterpreterParser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

bool InterpreterParser::checkHexadecimal(std::string &value) {
  if (value.starts_with("0x") || value.starts_with("0X")) {
    return true;
  }
  return false;
}

bool InterpreterParser::checkOctal(std::string &value) {
  if (value.starts_with("0") && value.size() != 1) {
    return true;
  }
  return false;
}

int64_t InterpreterParser::parseIntegerSuffix(std::string &value, int base) {
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
    throwInterParserException(
        "integer literal is too large to be represented in integer type");
    return -1;
  }
}

double InterpreterParser::parseFloatingSuffix(std::string &value, int base) {
  try {
    if (value.ends_with("f") || value.ends_with("F")) {
      return std::stof(value, nullptr);
    } else if (value.ends_with("l") || value.ends_with("L")) {
      return stold(value, nullptr);
    } else {
      return stod(value, nullptr);
    }
  } catch (std::out_of_range e) {
    throwInterParserException("magnitude of floating-point constant too large");
    return -0;
  }
}

std::string InterpreterParser::checkUnaryOperatorType(TokenType type,
                                                      Expr *right) {
  if (type == NOT) {
    return "i64";
  }
  return right->getType();
}

std::string InterpreterParser::checkBinaryOperatorType(
    TokenType type, std::unique_ptr<Expr> &left, std::unique_ptr<Expr> &right) {
  if (type == AND_OP || type == OR_OP) {
    return "i64";
  }
  if (left->getType() == right->getType()) {
    left = std::make_unique<ImplicitCastExpr>(left->getType(), std::move(left));
    right =
        std::make_unique<ImplicitCastExpr>(right->getType(), std::move(right));
    return left->getType();
  }
  if (left->getType() == "f64" || right->getType() == "f64") {
    left = std::make_unique<ImplicitCastExpr>("f64", std::move(left));
    right = std::make_unique<ImplicitCastExpr>("f64", std::move(right));
    return "f64";
  }
  return "i64";
}

std::string InterpreterParser::checkShiftOperatorType(
    TokenType type, std::unique_ptr<Expr> &left, std::unique_ptr<Expr> &right) {
  if (left->getType() == "f64" || right->getType() == "f64") {
    throwInterParserException(
        fstr("invalid operands to binary expression ('{}' and '{}')",
             left->getType(), right->getType()));
  }
  return "i64";
}

/**
 * parse Expr
 */

std::unique_ptr<Expr> InterpreterParser::parseIntegerLiteral() {
  auto value_str = previous().value;
  int64_t value;
  if (checkHexadecimal(value_str)) {
    value_str.erase(0, 2);
    value = parseIntegerSuffix(value_str, 16);
  } else if (checkOctal(value_str)) {
    value_str.erase(0, 1);
    value = parseIntegerSuffix(value_str, 8);
  } else if (value_str.starts_with("'")) {
    value_str.erase(0, 1); // remove leading '
    value_str.pop_back();  // remove suffix '
    int value = value_str[0];
    return std::make_unique<CharacterLiteral>(value);
  } else {
    value = parseIntegerSuffix(value_str, 10);
  }
  return std::make_unique<IntegerLiteral>(value, "i64");
}

std::unique_ptr<Expr> InterpreterParser::parseFloatingLiteral() {
  auto value_str = previous().value;
  auto value = parseFloatingSuffix(value_str, 10);
  return std::make_unique<FloatingLiteral>(value, "f64");
}

std::unique_ptr<Expr> InterpreterParser::parseConstant() {
  if (match(INTEGER)) {
    return parseIntegerLiteral();
  } else if (match(FLOATING)) {
    return parseFloatingLiteral();
  } else {
    throwInterParserException(fstr("not implemented type!"));
    return nullptr;
  }
}

std::unique_ptr<Expr> InterpreterParser::parsePrimaryExpression() {
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
    std::string type;
    if (peek().type != LP) { /// variable
      /// if local variable table not found, turn to global variable table
      type = varTable[name];
      if (type == "") {
        type = globalVarTable[name];
      }
      if (type == "") {
        throwInterParserException(fstr("identifier '{}' not found", name));
      }
      return std::make_unique<DeclRefExpr>(
          std::make_unique<VarDecl>(std::move(name), std::move(type)));
    } else { /// function call
      auto fp = funcTable[name];
      type = fp.retType;
      if (type == "") {
        throwInterParserException(
            fstr("implicit declaration of function '{}' is invalid", name));
      }

      std::string funcName = name;
      std::string retType = funcTable[name].retType;
      std::vector<std::unique_ptr<ParmVarDecl>> params;
      for (auto &param : funcTable[name].params) {
        std::string paramName = "";
        params.push_back(std::make_unique<ParmVarDecl>(paramName, param));
      }

      return std::make_unique<DeclRefExpr>(std::make_unique<FunctionDecl>(
          std::make_unique<FunctionProto>(funcName, retType, params)));
    }
  }
  if (match(LP)) {
    auto expr = parseExpression();
    consume(RP, "expected ')'");
    expr = std::make_unique<ParenExpr>(std::move(expr));
    return expr;
  }
  throwInterParserException("parse primary expression error");
  return nullptr;
}

std::unique_ptr<Expr> InterpreterParser::parsePostfixExpression() {
  auto expr = parsePrimaryExpression();
  /// parse function call
  if (match(LP)) {
    auto func =
        std::make_unique<DeclRefExpr>(dynamic_cast<DeclRefExpr *>(expr.get()));
    auto f = dynamic_cast<FunctionDecl *>(func->decl.get());
    if (f == nullptr) {
      throwInterParserException("error when parsing function call");
    }
    size_t idx = 0;
    std::vector<std::unique_ptr<Expr>> args;
    if (!check(RP)) {
      do {
        if (idx == f->proto->params.size()) {
          throwInterParserException(
              fstr("too many arguments to function call, expected {}",
                   f->proto->params.size()));
        }
        auto arg = parseExpression();
        arg = std::make_unique<ImplicitCastExpr>(
            f->proto->params[idx]->getType(), std::move(arg));
        idx++;
        args.push_back(std::move(arg));
      } while (match(COMMA));
    }
    consume(RP, "expect ')' after arguments");
    return std::make_unique<CallExpr>(std::move(func), std::move(args));
  }
  if (match({INC_OP, DEC_OP})) {
    auto op = previous();
    auto type = checkUnaryOperatorType(op.type, expr.get());
    return std::make_unique<UnaryOperator>(op, std::move(expr), std::move(type),
                                           POSTFIX);
  }
  return expr;
}

std::unique_ptr<Expr> InterpreterParser::parseUnaryExpression() {
  if (match({ADD, NOT, SUB})) {
    auto op = previous();
    auto expr = parseCastExpression();
    auto type = checkUnaryOperatorType(op.type, expr.get());
    return std::make_unique<UnaryOperator>(op, std::move(expr), std::move(type),
                                           PREFIX);
  }
  if (match({INC_OP, DEC_OP})) {
    auto op = previous();
    auto expr = parseUnaryExpression();
    auto type = checkUnaryOperatorType(op.type, expr.get());
    return std::make_unique<UnaryOperator>(op, std::move(expr), std::move(type),
                                           PREFIX);
  }
  return parsePostfixExpression();
}

std::unique_ptr<Expr> InterpreterParser::parseCastExpression() {
  if (match(LP)) {
    if (match({I64, F64})) {
      auto type = previous().value;
      consume(RP, "expected ')'");
      auto expr = parseCastExpression();
      return std::make_unique<CastExpr>(type, std::move(expr));
    }
    throwInterParserException(
        "support cast expression only type 'i64' and 'f64'");
  }
  return parseUnaryExpression();
}

std::unique_ptr<Expr> InterpreterParser::parseMultiplicativeExpression() {
  auto expr = parseCastExpression();
  while (match({MUL, DIV, MOD})) {
    auto op = previous();
    auto right = parseCastExpression();
    auto type = checkBinaryOperatorType(op.type, expr, right);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> InterpreterParser::parseAdditiveExpression() {
  auto expr = parseMultiplicativeExpression();
  while (match({ADD, SUB})) {
    auto op = previous();
    auto right = parseMultiplicativeExpression();
    auto type = checkBinaryOperatorType(op.type, expr, right);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> InterpreterParser::parseShiftExpression() {
  auto expr = parseAdditiveExpression();
  while (match({LEFT_OP, RIGHT_OP})) {
    auto op = previous();
    auto right = parseAdditiveExpression();
    auto type = checkShiftOperatorType(op.type, expr, right);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> InterpreterParser::parseRelationalExpression() {
  auto expr = parseShiftExpression();
  while (match({LE_OP, GE_OP, LA, RA})) {
    auto op = previous();
    auto right = parseShiftExpression();
    auto type = checkBinaryOperatorType(op.type, expr, right);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> InterpreterParser::parseEqualityExpression() {
  auto expr = parseRelationalExpression();
  while (match({EQ_OP, NE_OP})) {
    auto op = previous();
    auto right = parseRelationalExpression();
    auto type = checkBinaryOperatorType(op.type, expr, right);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> InterpreterParser::parseLogicalAndExpression() {
  auto expr = parseEqualityExpression();
  while (match(AND_OP)) {
    auto op = previous();
    auto right = parseEqualityExpression();
    auto type = checkBinaryOperatorType(op.type, expr, right);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> InterpreterParser::parseLogicalOrExpression() {
  auto expr = parseLogicalAndExpression();
  while (match(OR_OP)) {
    auto op = previous();
    auto right = parseLogicalAndExpression();
    auto type = checkBinaryOperatorType(op.type, expr, right);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> InterpreterParser::parseAssignmentExpression() {
  auto expr = parseLogicalOrExpression();
  if (match(EQUAL)) {
    auto token = previous();
    auto right = parseAssignmentExpression();
    if (expr->getType() != right->getType()) {
      right =
          std::make_unique<ImplicitCastExpr>(expr->getType(), std::move(right));
    }
    return std::make_unique<BinaryOperator>(token, std::move(expr),
                                            std::move(right), right->getType());
  }
  return expr;
}

std::unique_ptr<Expr> InterpreterParser::parseExpression() {
  return parseAssignmentExpression();
}

/**
 * parse Stmt
 */

std::unique_ptr<Stmt> InterpreterParser::parseExpressionStatement() {
  std::unique_ptr<Expr> expr = nullptr;
  if (!match(SEMI)) {
    expr = parseExpression();
    consume(SEMI, "expected ';' after expression");
  }
  return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Stmt> InterpreterParser::parseReturnStatement() {
  consume(RETURN, "expected 'return'");
  std::unique_ptr<Expr> expr = nullptr;
  if (!match(SEMI)) {
    expr = parseExpression();
    consume(SEMI, "expected ';' after expression");
  }
  return std::make_unique<ReturnStmt>(std::move(expr));
}

std::unique_ptr<Stmt> InterpreterParser::parseIterationStatement() {
  if (match(WHILE)) {
    consume(LP, "expect '(' after 'while'");
    auto expr = parseExpression();
    consume(RP, "expect ')'");
    auto stmt = parseStatement();
    return std::make_unique<WhileStmt>(std::move(expr), std::move(stmt));
  } else if (match(FOR)) {
    consume(LP, "expect '(' after 'for'");
    std::unique_ptr<Stmt> init;
    if (check({I64, F64})) {
      init = parseDeclarationStatement();
    } else {
      throwInterParserException("not support expression statement now!");
    }
    auto cond = parseExpression();
    consume(SEMI, "expected ';' after expression");
    auto update = parseExpression();
    consume(RP, "expect ')'");
    auto body = parseStatement();

    auto _init = dynamic_cast<DeclStmt *>(init.get());
    varTable.erase(_init->decl->getName());
    return std::make_unique<ForStmt>(std::make_unique<DeclStmt>(_init),
                                     std::move(cond), std::move(update),
                                     std::move(body));
  }
  throwInterParserException("error in iteration statement");
  return nullptr;
}

std::unique_ptr<Stmt> InterpreterParser::parseSelectionStatement() {
  if (match(IF)) {
    consume(LP, "expect '(' after 'if'");
    auto expr = parseExpression();
    consume(RP, "expect ')'");
    std::unique_ptr<Stmt> thenStmt, elseStmt;
    thenStmt = parseStatement();
    if (match(ELSE)) {
      elseStmt = parseStatement();
    }
    return std::make_unique<IfStmt>(std::move(expr), std::move(thenStmt),
                                    std::move(elseStmt));
  }
  throwInterParserException("error in selection statement");
  return nullptr;
}

std::unique_ptr<Stmt> InterpreterParser::parseDeclarationStatement() {
  advance();
  auto type = previous().value;
  if (match(IDENTIFIER)) {
    auto name = previous().value;
    auto decl = parseVariableDeclaration(type, name, LOCAL);
    return std::make_unique<DeclStmt>(std::move(decl));
  }
  throwInterParserException("expected identifier");
  return nullptr;
}

std::unique_ptr<Stmt> InterpreterParser::parseCompoundStatement() {
  consume(LC, "expected function body after function declarator");
  std::vector<std::unique_ptr<Stmt>> stmts;
  while (!check(RC) && current.type != _EOF) {
    auto stmt = parseStatement();
    stmts.push_back(std::move(stmt));
  }
  consume(RC, "expected '}'");
  return std::make_unique<CompoundStmt>(std::move(stmts));
}

std::unique_ptr<Stmt> InterpreterParser::parseStatement() {
  if (check(RETURN)) {
    return parseReturnStatement();
  }
  if (check({VOID, I64, F64})) {
    return parseDeclarationStatement();
  }
  if (check(IF)) {
    return parseSelectionStatement();
  }
  /// support while and for statement
  if (check({WHILE, FOR})) {
    return parseIterationStatement();
  }
  /// for other use of `parseCompoundStatement()`,
  /// use `check()` instead of `match()`
  if (check(LC)) {
    return parseCompoundStatement();
  }
  return parseExpressionStatement();
}

std::pair<std::unique_ptr<ExprStmt>, bool>
InterpreterParser::parseExpressionOrExpressionStatement() {
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

/**
 * internal parse
 */

std::pair<std::string, bool> InterpreterParser::parseDeclarationSpecifiers() {
  std::string spec;
  bool isExtern = false;
  if (match(EXTERN)) {
    isExtern = true;
  }
  if (match({VOID, I64, F64})) {
    spec = previous().value;
  };
  return {spec, isExtern};
  throwInterParserException("expected type specifier");
  return {"", false};
}

std::string InterpreterParser::parseDeclarator() {
  if (match(IDENTIFIER)) {
    return previous().value;
  }
  throwInterParserException("expected identifier");
  return "";
}

std::vector<std::unique_ptr<ParmVarDecl>>
InterpreterParser::parseFunctionParameters() {
  std::vector<std::unique_ptr<ParmVarDecl>> params;
  if (!check(RP)) {
    do {
      if (params.size() >= 255) {
        throwInterParserException("can't have more than 255 parameters");
      }
      auto [type, flag] = parseDeclarationSpecifiers();
      auto name = parseDeclarator();
      varTable[name] = type;
      params.push_back(
          std::make_unique<ParmVarDecl>(std::move(name), std::move(type)));

    } while (match(COMMA));
  }
  return params;
}

std::string InterpreterParser::genFuncType(
    std::string &&retTy, std::vector<std::unique_ptr<ParmVarDecl>> &params) {
  std::string funcType = retTy + " (";
  for (size_t i = 0; i < params.size(); i++) {
    funcType += params[i]->getType();
    if (i != params.size() - 1) {
      funcType += ", ";
    }
  }
  funcType += ")";
  return funcType;
}

/**
 * parse Decl
 */

std::unique_ptr<Decl>
InterpreterParser::parseVariableDeclaration(std::string &type,
                                            std::string &name, VarScope scope) {
  std::unique_ptr<Expr> init;
  if (scope == GLOBAL) {
    if (globalVarTable.find(name) != globalVarTable.end()) {
      throwInterParserException(fstr("redefinition of '{}'", name));
    }
    globalVarTable[name] = type;
    /// for global variable, set default value
    std::unique_ptr<Expr> zero;
    if (type == "i64") {
      zero = std::make_unique<IntegerLiteral>(0, "i64");
    } else if (type == "f64") {
      zero = std::make_unique<FloatingLiteral>(0, "f64");
    } else {
      throwInterParserException("not supported type");
    }
    init = (match(EQUAL) ? parseConstant() : std::move(zero));
  } else {
    if (varTable.find(name) != varTable.end()) {
      throwInterParserException(fstr("redefinition of '{}'", name));
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

std::unique_ptr<Decl>
InterpreterParser::parseFunctionDeclaration(std::string &retTy,
                                            std::string &name, bool isExtern) {
  clearVarTable();
  std::vector<std::unique_ptr<ParmVarDecl>> params = parseFunctionParameters();
  consume(RP, "expected parameter declarator");
  std::string funcType = genFuncType(std::move(retTy), params);

  /// store in funcTable
  std::vector<std::string> paramsTy;
  for (auto &param : params) {
    paramsTy.push_back(param->getType());
  }
  FunctionParams fp(name, retTy, paramsTy);
  funcTable[name] = fp;

  std::unique_ptr<Stmt> body = nullptr;
  /// if match SEMI, body in null, otherwise, parse the function body
  if (!match(SEMI)) {
    body = parseCompoundStatement();
  }
  FuncKind kind =
      (isExtern ? EXTERN_FUNC : (body == nullptr ? DECLARATION : DEFINITION));
  return std::make_unique<FunctionDecl>(
      std::make_unique<FunctionProto>(std::move(name), std::move(funcType),
                                      std::move(params)),
      std::move(body), kind);
}

std::unique_ptr<Decl> InterpreterParser::parseExternalDeclaration() {
  auto [type, flag] = parseDeclarationSpecifiers();
  auto name = parseDeclarator();
  if (match(LP)) {
    return parseFunctionDeclaration(type, name, flag);
  } else {
    return parseVariableDeclaration(type, name, GLOBAL);
  }
}

InterpreterParser::parse_t InterpreterParser::parse() {
  parse_t translationUnit;

  advance();
  // std::cout << fstr("parse {}\n", current.toString());
  std::string type, name;
  bool isExtern = false;
  if (match(EXTERN)) {
    isExtern = true;
  }
  /// declaration
  if (match({VOID, I64, F64})) {
    // std::cout << fstr("parse declaration\n");
    type = previous().value;
    if (match(IDENTIFIER)) {
      name = previous().value;
    } else {
      throwInterParserException("invalid expression");
    }
    if (match(LP)) {
      translationUnit = parseFunctionDeclaration(type, name, isExtern);
    } else {
      translationUnit = parseVariableDeclaration(type, name, LOCAL);
    }
  } else if (check({IF, WHILE, FOR, LC})) {
    // std::cout << fstr("parse statement\n");
    translationUnit = parseStatement();
  } else {
    auto [stmt, flag] = parseExpressionOrExpressionStatement();
    if (flag) {
      // std::cout << fstr("parse expression statement\n");
      translationUnit = std::move(stmt);
    } else {
      // std::cout << fstr("parse expression\n");
      translationUnit = std::move(stmt->expr);
    }
  }
  return translationUnit;
}

} // namespace toyc
