//! BaseParser implementation

#include <AST/AST.h>
#include <Lexer/Token.h>
#include <Parser/Parser.h>
#include <Util.h>

#include <memory>
#include <tuple>
#include <vector>

namespace toyc {

/**
 * @brief Base Parser
 *
 */

Token BaseParser::advance() {
  prev = current;
  for (;;) {
    current = lexer.scanToken();
    if (current.type != ERROR) {
      break;
    }
    throwParserException("error at parse");
  }
  return prev;
}

Token BaseParser::consume(TokenTy type, std::string &message) {
  if (current.type == type) {
    return advance();
  }
  throwParserException(message);
  return Token(ERROR, "");
}

Token BaseParser::consume(TokenTy type, std::string &&message) {
  if (current.type == type) {
    return advance();
  }
  throwParserException(message);
  return Token(ERROR, "");
}

bool BaseParser::check(std::initializer_list<TokenTy> types) {
  for (TokenTy type : types) {
    if (type == _EOF) {
      return false;
    }
    if (peek().type == type) {
      return true;
    }
  }
  return false;
}

bool BaseParser::check(TokenTy type) {
  if (type == _EOF) {
    return false;
  }
  return peek().type == type;
}

bool BaseParser::match(std::initializer_list<TokenTy> types) {
  for (TokenTy type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

bool BaseParser::match(TokenTy type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

int64_t BaseParser::parseIntegerSuffix(std::string &value, int base) {
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

double BaseParser::parseFloatingSuffix(std::string &value, int base) {
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

/**
 * parse Expr
 */

ExprPtr BaseParser::parseIntegerLiteral() {
  std::string value_str = previous().value;
  int64_t value;
  if (actions.checkHexadecimal(value_str)) {
    value_str.erase(0, 2);
    value = parseIntegerSuffix(value_str, 16);
  } else if (actions.checkOctal(value_str)) {
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

ExprPtr BaseParser::parseFloatingLiteral() {
  std::string value_str = previous().value;
  double value = parseFloatingSuffix(value_str, 10);
  return std::make_unique<FloatingLiteral>(value, "f64");
}

ExprPtr BaseParser::parsePrimaryExpression() {
  if (match(INTEGER)) {
    return parseIntegerLiteral();
  }
  if (match(FLOATING)) {
    return parseFloatingLiteral();
  }
  if (match(STRING)) {
    std::string value = previous().value;
    /// add a terminator '\0' size
    std::string type = makeString("char[{}]", value.size() + 1);
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
        throwParserException(makeString("identifier '{}' not found", name));
      }
      return std::make_unique<DeclRefExpr>(
          std::make_unique<VarDecl>(std::move(name), std::move(type)));
    } else { /// function call
      auto fp = funcTable[name];
      type = fp.retType;
      if (type == "") {
        throwParserException(
            makeString("implicit declaration of function '{}' is invalid", name));
      }

      std::string funcName = name;
      std::string retType = funcTable[name].retType;
      std::vector<std::unique_ptr<ParmVarDecl>> params;
      for (auto &param : funcTable[name].params) {
        std::string paramName = "";
        params.push_back(std::make_unique<ParmVarDecl>(paramName, param));
      }

      return std::make_unique<DeclRefExpr>(
          std::make_unique<FunctionDecl>(std::make_unique<FunctionProto>(
              funcName, retType, std::move(params), 0)));
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

ExprPtr BaseParser::parsePostfixExpression() {
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
    std::vector<ExprPtr> args;
    if (!check(RP)) {
      do {
        if (idx == f->proto->params.size()) {
          throwParserException(
              makeString("too many arguments to function call, expected {}",
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
    if (expr->assignable() == false) {
      throwParserException("expression is not assignable");
    }
    Token op = previous();
    std::string type = actions.checkUnaryOperatorType(expr, op.type);
    return std::make_unique<UnaryOperator>(op, std::move(expr), std::move(type),
                                           POSTFIX);
  }
  return expr;
}

ExprPtr BaseParser::parseUnaryExpression() {
  /// prefix unary operator
  if (match({ADD, NOT, SUB})) {
    Token op = previous();
    auto expr = parseUnaryExpression();
    std::string type = actions.checkUnaryOperatorType(expr, op.type);
    return std::make_unique<UnaryOperator>(op, std::move(expr), std::move(type),
                                           PREFIX);
  }
  /// prefix unary operator (assignable)
  if (match({INC_OP, DEC_OP})) {
    Token op = previous();
    auto expr = parseUnaryExpression();
    if (expr->assignable() == false) {
      throwParserException("expression is not assignable");
    }
    std::string type = actions.checkUnaryOperatorType(expr, op.type);
    return std::make_unique<UnaryOperator>(op, std::move(expr), std::move(type),
                                           PREFIX);
  }
  return parsePostfixExpression();
}

ExprPtr BaseParser::parseMultiplicativeExpression() {
  auto expr = parseUnaryExpression();
  while (match({MUL, DIV, MOD})) {
    Token op = previous();
    auto right = parseUnaryExpression();
    std::string type = actions.checkBinaryOperatorType(expr, right, op.type);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

ExprPtr BaseParser::parseAdditiveExpression() {
  auto expr = parseMultiplicativeExpression();
  while (match({ADD, SUB})) {
    Token op = previous();
    auto right = parseMultiplicativeExpression();
    std::string type = actions.checkBinaryOperatorType(expr, right, op.type);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

ExprPtr BaseParser::parseShiftExpression() {
  auto expr = parseAdditiveExpression();
  while (match({LEFT_OP, RIGHT_OP})) {
    Token op = previous();
    auto right = parseAdditiveExpression();
    std::string type = actions.checkShiftOperatorType(expr, right, op.type);
    if (type == "") {
      throwParserException(
          makeString("invalid operands to binary expression ('{}' and '{}')",
               expr->getType(), right->getType()));
    }
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

ExprPtr BaseParser::parseRelationalExpression() {
  auto expr = parseShiftExpression();
  while (match({LE_OP, GE_OP, LA, RA})) {
    Token op = previous();
    auto right = parseShiftExpression();
    std::string type = actions.checkBinaryOperatorType(expr, right, op.type);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

ExprPtr BaseParser::parseEqualityExpression() {
  auto expr = parseRelationalExpression();
  while (match({EQ_OP, NE_OP})) {
    Token op = previous();
    auto right = parseRelationalExpression();
    std::string type = actions.checkBinaryOperatorType(expr, right, op.type);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

ExprPtr BaseParser::parseLogicalAndExpression() {
  auto expr = parseEqualityExpression();
  while (match(AND_OP)) {
    Token op = previous();
    auto right = parseEqualityExpression();
    std::string type = actions.checkBinaryOperatorType(expr, right, op.type);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

ExprPtr BaseParser::parseLogicalOrExpression() {
  auto expr = parseLogicalAndExpression();
  while (match(OR_OP)) {
    Token op = previous();
    auto right = parseLogicalAndExpression();
    std::string type = actions.checkBinaryOperatorType(expr, right, op.type);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

ExprPtr BaseParser::parseAssignmentExpression() {
  auto expr = parseLogicalOrExpression();
  if (match(EQUAL)) {
    Token token = previous();
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

ExprPtr BaseParser::parseExpression() { return parseAssignmentExpression(); }

/**
 * parse Stmt
 */

StmtPtr BaseParser::parseExpressionStatement() {
  ExprPtr expr = nullptr;
  if (!match(SEMI)) {
    expr = parseExpression();
    consume(SEMI, "expected ';' after expression");
  }
  return std::make_unique<ExprStmt>(std::move(expr));
}

StmtPtr BaseParser::parseReturnStatement() {
  consume(RETURN, "expected 'return'");
  ExprPtr expr = nullptr;
  if (!match(SEMI)) {
    expr = parseExpression();
    consume(SEMI, "expected ';' after expression");
  }
  return std::make_unique<ReturnStmt>(std::move(expr));
}

StmtPtr BaseParser::parseIterationStatement() {
  if (match(WHILE)) {
    consume(LP, "expect '(' after 'while'");
    auto expr = parseExpression();
    consume(RP, "expect ')'");
    auto stmt = parseStatement();
    return std::make_unique<WhileStmt>(std::move(expr), std::move(stmt));
  } else if (match(FOR)) {
    consume(LP, "expect '(' after 'for'");
    StmtPtr init;
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

StmtPtr BaseParser::parseSelectionStatement() {
  if (match(IF)) {
    consume(LP, "expect '(' after 'if'");
    auto expr = parseExpression();
    consume(RP, "expect ')'");
    StmtPtr thenStmt, elseStmt;
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

StmtPtr BaseParser::parseDeclarationStatement() {
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

StmtPtr BaseParser::parseCompoundStatement() {
  consume(LC, "expected function body after function declarator");
  std::vector<StmtPtr> stmts;
  while (!check(RC) && current.type != _EOF) {
    auto stmt = parseStatement();
    stmts.push_back(std::move(stmt));
  }
  consume(RC, "expected '}'");
  return std::make_unique<CompoundStmt>(std::move(stmts));
}

StmtPtr BaseParser::parseStatement() {
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

std::pair<std::string, bool> BaseParser::parseDeclarationSpecifiers() {
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

std::string BaseParser::parseDeclarator() {
  if (match(IDENTIFIER)) {
    return previous().value;
  }
  throwParserException("expected identifier");
  return "";
}

std::vector<std::unique_ptr<ParmVarDecl>>
BaseParser::parseFunctionParameters() {
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
BaseParser::genFuncType(std::string &&retTy,
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

DeclPtr BaseParser::parseVariableDeclaration(std::string type, std::string name,
                                             VarScope scope) {
  ExprPtr init;
  if (scope == GLOBAL) {
    if (globalVarTable.find(name) != globalVarTable.end()) {
      throwParserException(makeString("redefinition of '{}'", name));
    }
    /// for global variable, set default value
    ExprPtr zero;
    if (type == "i64") {
      zero = std::make_unique<IntegerLiteral>(0, "i64");
    } else if (type == "f64") {
      zero = std::make_unique<FloatingLiteral>(0, "f64");
    } else {
      throwParserException("not supported type");
    }
    /// TODO: support parse minus number
    init = (match(EQUAL) ? parseAssignmentExpression() : std::move(zero));
    if (!init->isConstant()) {
      throwParserException(
          "initializer element is not a compile-time constant");
    }
    globalVarTable[name] = type;
  } else {
    if (varTable.find(name) != varTable.end()) {
      throwParserException(makeString("redefinition of '{}'", name));
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

DeclPtr BaseParser::parseFunctionDeclaration(std::string retTy,
                                             std::string name, bool isExtern) {
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

  StmtPtr body = nullptr;
  /// if match SEMI, body in null, otherwise, parse the function body
  if (!match(SEMI)) {
    body = parseCompoundStatement();
  }
  FuncKind kind =
      (isExtern ? EXTERN_FUNC : (body == nullptr ? DECLARATION : DEFINITION));
  return std::make_unique<FunctionDecl>(
      std::make_unique<FunctionProto>(std::move(name), std::move(funcType),
                                      std::move(params), 0),
      std::move(body), kind);
}

DeclPtr BaseParser::parseExternalDeclaration() {
  auto [type, flag] = parseDeclarationSpecifiers();
  auto name = parseDeclarator();
  if (match(LP)) {
    return parseFunctionDeclaration(type, name, flag);
  } else {
    return parseVariableDeclaration(type, name, GLOBAL);
  }
}

/**
 * Parser
 */

std::unique_ptr<TranslationUnitDecl> Parser::parse() {
  std::vector<DeclPtr> decls;
  advance();
  while (current.type != _EOF) {
    auto decl = parseExternalDeclaration();
    decls.push_back(std::move(decl));
  }
  return std::make_unique<TranslationUnitDecl>(std::move(decls));
}

} // namespace toyc
