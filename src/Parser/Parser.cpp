//! Parser implementation

#include <AST/AST.h>
#include <CodeGen/CodeGen.h>
#include <Lexer/Token.h>
#include <Parser/Parser.h>
#include <Util.h>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace toyc {

Token Parser::advance() {
  prev = current;
  for (;;) {
    current = lexer.scanToken();
    if (current.type != ERROR) {
      break;
    }
    throwParserException("error at parse");
  }
  // debug("prev:{}, curent:{}", prev.toString(), current.toString());
  return prev;
}

Token Parser::consume(TokenType type, std::string &message) {
  if (current.type == type) {
    return advance();
  }
  throwParserException(message);
  return Token(ERROR, "");
}

Token Parser::consume(TokenType type, std::string &&message) {
  if (current.type == type) {
    return advance();
  }
  throwParserException(message);
  return Token(ERROR, "");
}

bool Parser::check(std::initializer_list<TokenType> types) {
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

bool Parser::check(TokenType type) {
  if (type == _EOF) {
    return false;
  }
  return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
  for (TokenType type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

bool Parser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

bool Parser::checkHexadecimal(std::string &value) {
  if (value.starts_with("0x") || value.starts_with("0X")) {
    return true;
  }
  return false;
}

bool Parser::checkOctal(std::string &value) {
  if (value.starts_with("0") && value.size() != 1) {
    return true;
  }
  return false;
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
    throwParserException(
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
    throwParserException("magnitude of floating-point constant too large");
    return -0;
  }
}

std::string Parser::checkUnaryOperatorType(TokenType type, Expr *right) {
  if (type == NOT) {
    return "i64";
  }
  return right->getType();
}

std::string Parser::checkBinaryOperatorType(TokenType type,
                                            std::unique_ptr<Expr> &left,
                                            std::unique_ptr<Expr> &right) {
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
    std::string type;
    if (peek().type != LP) { /// variable
      /// if local variable table not found, turn to global variable table
      type = varTable[name];
      if (type == "") {
        type = globalVarTable[name];
      }
      if (type == "") {
        throwParserException(fstr("identifier '{}' not found", name));
      }
      return std::make_unique<DeclRefExpr>(
          std::make_unique<VarDecl>(std::move(name), std::move(type)));
    } else { /// function call
      type = funcTable[name].first;
      if (type == "") {
        throwParserException(
            fstr("implicit declaration of function '{}' is invalid", name));
      }

      std::string funcName = name;
      std::string retType = funcTable[name].first;
      std::vector<std::unique_ptr<ParmVarDecl>> params;
      for (auto &param : funcTable[name].second) {
        std::string paramName = "";
        params.push_back(std::make_unique<ParmVarDecl>(paramName, param));
      }

      return std::make_unique<DeclRefExpr>(
          std::make_unique<FunctionDecl>(funcName, retType, params));
    }
  }
  if (match(LP)) {
    auto expr = parseExpression();
    consume(RP, "expected ')'");
    expr = std::make_unique<ParenExpr>(std::move(expr));
    return expr;
  }
  throwParserException("parse primary expression error");
  return nullptr;
}

std::unique_ptr<Expr> Parser::parsePostfixExpression() {
  auto expr = parsePrimaryExpression();
  /// parse function call
  if (match(LP)) {
    auto func =
        std::make_unique<DeclRefExpr>(dynamic_cast<DeclRefExpr *>(expr.get()));
    auto f = dynamic_cast<FunctionDecl *>(func->decl.get());
    if (f == nullptr) {
      throwParserException("error when parsing function call");
    }
    size_t idx = 0;
    std::vector<std::unique_ptr<Expr>> args;
    if (!check(RP)) {
      do {
        if (idx == f->params.size()) {
          throwParserException(
              fstr("too many arguments to function call, expected {}",
                   f->params.size()));
        }
        auto arg = parseExpression();
        arg = std::make_unique<ImplicitCastExpr>(f->params[idx]->getType(),
                                                 std::move(arg));
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

std::unique_ptr<Expr> Parser::parseUnaryExpression() {
  if (match({ADD, NOT, SUB, INC_OP, DEC_OP})) {
    auto op = previous();
    auto expr = parseUnaryExpression();
    auto type = checkUnaryOperatorType(op.type, expr.get());
    return std::make_unique<UnaryOperator>(op, std::move(expr),
                                           std::move(type), PREFIX);
  }
  return parsePostfixExpression();
}

std::unique_ptr<Expr> Parser::parseMultiplicativeExpression() {
  auto expr = parseUnaryExpression();
  while (match({MUL, DIV, MOD})) {
    auto op = previous();
    auto right = parseUnaryExpression();
    auto type = checkBinaryOperatorType(op.type, expr, right);
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
    auto type = checkBinaryOperatorType(op.type, expr, right);
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
    auto type = checkBinaryOperatorType(op.type, expr, right);
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
    auto type = checkBinaryOperatorType(op.type, expr, right);
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
    auto type = checkBinaryOperatorType(op.type, expr, right);
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
    auto type = checkBinaryOperatorType(op.type, expr, right);
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
    if (expr->getType() != right->getType()) {
      right =
          std::make_unique<ImplicitCastExpr>(expr->getType(), std::move(right));
    }
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
  std::unique_ptr<Expr> expr = nullptr;
  if (!match(SEMI)) {
    expr = parseExpression();
    consume(SEMI, "expected ';' after expression");
  }
  return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseReturnStatement() {
  consume(RETURN, "expected 'return'");
  std::unique_ptr<Expr> expr = nullptr;
  if (!match(SEMI)) {
    expr = parseExpression();
    consume(SEMI, "expected ';' after expression");
  }
  return std::make_unique<ReturnStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseIterationStatement() {
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
      throwParserException("not support expression statement now!");
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
  throwParserException("error in iteration statement");
  return nullptr;
}

std::unique_ptr<Stmt> Parser::parseSelectionStatement() {
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
  throwParserException("error in selection statement");
  return nullptr;
}

std::unique_ptr<Stmt> Parser::parseDeclarationStatement() {
  advance();
  auto type = previous().value;
  if (match(IDENTIFIER)) {
    auto name = previous().value;
    auto decl = parseVariableDeclaration(type, name, LOCAL);
    return std::make_unique<DeclStmt>(std::move(decl));
  }
  throwParserException("expected identifier");
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

/**
 * internal parse
 */

std::pair<std::string, bool> Parser::parseDeclarationSpecifiers() {
  std::string spec;
  bool isExtern = false;
  if (match(EXTERN)) {
    isExtern = true;
  }
  if (match({VOID, I64, F64})) {
    spec = previous().value;
  };
  return {spec, isExtern};
  throwParserException("expected type specifier");
  return {"", false};
}

std::string Parser::parseDeclarator() {
  if (match(IDENTIFIER)) {
    return previous().value;
  }
  throwParserException("expected identifier");
  return "";
}

std::vector<std::unique_ptr<ParmVarDecl>> Parser::parseFunctionParameters() {
  std::vector<std::unique_ptr<ParmVarDecl>> params;
  if (!check(RP)) {
    do {
      if (params.size() >= 255) {
        throwParserException("can't have more than 255 parameters");
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

std::string
Parser::genFuncType(std::string &&retTy,
                    std::vector<std::unique_ptr<ParmVarDecl>> &params) {
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

std::unique_ptr<Decl> Parser::parseVariableDeclaration(std::string &type,
                                                       std::string &name,
                                                       VarScope scope) {
  if (scope == GLOBAL) {
    if (globalVarTable.find(name) != globalVarTable.end()) {
      throwParserException(fstr("redefinition of '{}'", name));
    }
    globalVarTable[name] = type;
  } else {
    if (varTable.find(name) != varTable.end()) {
      throwParserException(fstr("redefinition of '{}'", name));
    }
    varTable[name] = type;
  }

  std::unique_ptr<Expr> init =
      (match(EQUAL) ? parseAssignmentExpression() : nullptr);
  if (init != nullptr && type != init->getType()) {
    init = std::make_unique<ImplicitCastExpr>(type, std::move(init));
  }
  std::unique_ptr<VarDecl> decl = std::make_unique<VarDecl>(
      std::move(name), std::move(type), std::move(init), scope);
  consume(SEMI, "expected ';' after declaration");
  return decl;
}

std::unique_ptr<Decl> Parser::parseFunctionDeclaration(std::string &retTy,
                                                       std::string &name,
                                                       bool isExtern) {
  clearVarTable();
  std::vector<std::unique_ptr<ParmVarDecl>> params = parseFunctionParameters();
  consume(RP, "expected parameter declarator");
  std::string funcType = genFuncType(std::move(retTy), params);

  /// store in funcTable
  std::vector<std::string> paramsTy;
  for (auto &param : params) {
    paramsTy.push_back(param->getType());
  }
  funcTable[name] = std::make_pair(retTy, paramsTy);

  std::unique_ptr<Stmt> body = nullptr;
  /// if match SEMI, body in null, otherwise, parse the function body
  if (!match(SEMI)) {
    body = parseCompoundStatement();
  }
  FuncKind kind =
      (isExtern ? EXTERN_FUNC : (body == nullptr ? DECLARATION : DEFINITION));
  return std::make_unique<FunctionDecl>(std::move(name), std::move(funcType),
                                        std::move(params), std::move(body),
                                        kind);
}

std::unique_ptr<Decl> Parser::parseExternalDeclaration() {
  auto [type, flag] = parseDeclarationSpecifiers();
  auto name = parseDeclarator();
  if (match(LP)) {
    return parseFunctionDeclaration(type, name, flag);
  } else {
    return parseVariableDeclaration(type, name, GLOBAL);
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

} // namespace toyc
