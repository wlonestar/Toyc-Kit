//! BaseParser implementation

#include "Lexer/Lexer.h"
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

auto BaseParser::Advance() -> Token {
  prev_ = current_;
  for (;;) {
    try {
      current_ = lexer_.ScanToken();
    } catch (LexerException e) {
      std::cerr << e.what() << "\n";
      lexer_.Advance();
      continue;
    }
    if (current_.type_ != ERROR) {
      break;
    }
    ThrowParserException("error at parse");
  }
  return prev_;
}

auto BaseParser::Consume(TokenTy type, std::string &message) -> Token {
  if (current_.type_ == type) {
    return Advance();
  }
  ThrowParserException(message);
  return {ERROR, ""};
}

auto BaseParser::Consume(TokenTy type, std::string &&message) -> Token {
  if (current_.type_ == type) {
    return Advance();
  }
  ThrowParserException(message);
  return {ERROR, ""};
}

auto BaseParser::Check(std::initializer_list<TokenTy> types) -> bool {
  for (TokenTy type : types) {
    if (type == _EOF) {
      return false;
    }
    if (Peek().type_ == type) {
      return true;
    }
  }
  return false;
}

auto BaseParser::Check(TokenTy type) -> bool {
  if (type == _EOF) {
    return false;
  }
  return Peek().type_ == type;
}

auto BaseParser::Match(std::initializer_list<TokenTy> types) -> bool {
  // for (TokenTy type : types) {
  //   if (Check(type)) {
  //     Advance();
  //     return true;
  //   }
  // }
  // return false;
  return std::ranges::any_of(types.begin(), types.end(), [&](auto type) {
    if (Check(type)) {
      Advance();
      return true;
    }
    return false;
  });
}

auto BaseParser::Match(TokenTy type) -> bool {
  if (Check(type)) {
    Advance();
    return true;
  }
  return false;
}

auto BaseParser::ParseIntegerSuffix(std::string &value, int base) -> int64_t {
  try {
    if (value.ends_with("llu") || value.ends_with("llU") ||
        value.ends_with("LLu") || value.ends_with("LLU") ||
        value.ends_with("ull") || value.ends_with("uLL") ||
        value.ends_with("Ull") || value.ends_with("ULL")) {
      return stoull(value, nullptr, base);
    }
    if (value.ends_with("ul") || value.ends_with("uL") ||
        value.ends_with("Ul") || value.ends_with("UL") ||
        value.ends_with("lu") || value.ends_with("lU") ||
        value.ends_with("Lu") || value.ends_with("LU")) {
      return stoul(value, nullptr, base);
    }
    if (value.ends_with("ll") || value.ends_with("LL")) {
      return stoll(value, nullptr, base);
    }
    if (value.ends_with("l") || value.ends_with("L")) {
      return stol(value, nullptr, base);
    }
    if (value.ends_with("u") || value.ends_with("U")) {
      return static_cast<unsigned int>(stoul(value, nullptr, base));
    }
    return stoi(value, nullptr, base);
  } catch (std::out_of_range e) {
    ThrowParserException(
        "integer literal is too large to be represented in integer type");
    return -1;
  }
}

auto BaseParser::ParseFloatingSuffix(std::string &value, int base) -> double {
  try {
    if (value.ends_with("f") || value.ends_with("F")) {
      return std::stof(value, nullptr);
    }
    if (value.ends_with("l") || value.ends_with("L")) {
      return stold(value, nullptr);
    }
    return stod(value, nullptr);
  } catch (std::out_of_range e) {
    ThrowParserException("magnitude of floating-point constant too large");
    return -0;
  }
}

/**
 * parse Expr
 */

auto BaseParser::ParseIntegerLiteral() -> ExprPtr {
  std::string value_str = Previous().value_;
  int64_t value;
  if (actions_.CheckHexadecimal(value_str)) {
    value_str.erase(0, 2);
    value = ParseIntegerSuffix(value_str, 16);
  } else if (actions_.CheckOctal(value_str)) {
    value_str.erase(0, 1);
    value = ParseIntegerSuffix(value_str, 8);
  } else if (value_str.starts_with("'")) {
    value_str.erase(0, 1); // remove leading '
    value_str.pop_back();  // remove suffix '
    int value = value_str[0];
    return std::make_unique<CharacterLiteral>(value);
  } else {
    value = ParseIntegerSuffix(value_str, 10);
  }
  return std::make_unique<IntegerLiteral>(value, "i64");
}

auto BaseParser::ParseFloatingLiteral() -> ExprPtr {
  std::string value_str = Previous().value_;
  double value = ParseFloatingSuffix(value_str, 10);
  return std::make_unique<FloatingLiteral>(value, "f64");
}

auto BaseParser::ParsePrimaryExpression() -> ExprPtr {
  if (Match(INTEGER)) {
    return ParseIntegerLiteral();
  }
  if (Match(FLOATING)) {
    return ParseFloatingLiteral();
  }
  if (Match(STRING)) {
    std::string value = Previous().value_;
    /// add a terminator '\0' size
    std::string type = makeString("char[{}]", value.size() + 1);
    return std::make_unique<StringLiteral>(std::move(value), std::move(type));
  }
  if (Match(IDENTIFIER)) {
    Token token = Previous();
    std::string name = token.value_;
    std::string type;
    if (Peek().type_ != LP) { /// variable
      /// if local variable table not found, turn to global variable table
      type = var_table_[name];
      if (type.empty()) {
        type = global_var_table_[name];
      }
      if (type.empty()) {
        ThrowParserException(makeString("identifier '{}' not found", name));
      }
      return std::make_unique<DeclRefExpr>(
          std::make_unique<VarDecl>(std::move(name), std::move(type)));
    }

    /// function call
    auto fp = func_table_[name];
    type = fp.ret_type_;
    if (type.empty()) {
      ThrowParserException(
          makeString("implicit declaration of function '{}' is invalid", name));
    }

    std::string func_name = name;
    std::string ret_type = func_table_[name].ret_type_;
    std::vector<std::unique_ptr<ParmVarDecl>> params;
    for (auto &param : func_table_[name].params_) {
      std::string param_name;
      params.push_back(std::make_unique<ParmVarDecl>(param_name, param));
    }

    return std::make_unique<DeclRefExpr>(
        std::make_unique<FunctionDecl>(std::make_unique<FunctionProto>(
            func_name, ret_type, std::move(params), 0)));
  }
  if (Match(LP)) {
    auto expr = ParseExpression();
    Consume(RP, "expected ')'");
    expr = std::make_unique<ParenExpr>(std::move(expr));
    return expr;
  }
  ThrowParserException("parse primary expression error");
  return nullptr;
}

auto BaseParser::ParsePostfixExpression() -> ExprPtr {
  auto expr = ParsePrimaryExpression();
  /// parse function call
  if (Match(LP)) {
    auto func =
        std::make_unique<DeclRefExpr>(dynamic_cast<DeclRefExpr *>(expr.get()));
    auto f = dynamic_cast<FunctionDecl *>(func->decl_.get());
    if (f == nullptr) {
      ThrowParserException("error when parsing function call");
    }
    size_t idx = 0;
    std::vector<ExprPtr> args;
    if (!Check(RP)) {
      do {
        if (idx == f->proto_->params_.size()) {
          ThrowParserException(
              makeString("too many arguments to function call, expected {}",
                         f->proto_->params_.size()));
        }
        auto arg = ParseExpression();
        arg = std::make_unique<ImplicitCastExpr>(
            f->proto_->params_[idx]->GetType(), std::move(arg));
        idx++;
        args.push_back(std::move(arg));
      } while (Match(COMMA));
    }
    Consume(RP, "expect ')' after arguments");
    return std::make_unique<CallExpr>(std::move(func), std::move(args));
  }
  if (Match({INC_OP, DEC_OP})) {
    if (!expr->Assignable()) {
      ThrowParserException("expression is not assignable");
    }
    Token op = Previous();
    std::string type = actions_.CheckUnaryOperatorType(expr, op.type_);
    return std::make_unique<UnaryOperator>(op, std::move(expr), std::move(type),
                                           POSTFIX);
  }
  return expr;
}

auto BaseParser::ParseUnaryExpression() -> ExprPtr {
  /// prefix unary operator
  if (Match({ADD, NOT, SUB})) {
    Token op = Previous();
    auto expr = ParseUnaryExpression();
    std::string type = actions_.CheckUnaryOperatorType(expr, op.type_);
    return std::make_unique<UnaryOperator>(op, std::move(expr), std::move(type),
                                           PREFIX);
  }
  /// prefix unary operator (assignable)
  if (Match({INC_OP, DEC_OP})) {
    Token op = Previous();
    auto expr = ParseUnaryExpression();
    if (!expr->Assignable()) {
      ThrowParserException("expression is not assignable");
    }
    std::string type = actions_.CheckUnaryOperatorType(expr, op.type_);
    return std::make_unique<UnaryOperator>(op, std::move(expr), std::move(type),
                                           PREFIX);
  }
  return ParsePostfixExpression();
}

auto BaseParser::ParseMultiplicativeExpression() -> ExprPtr {
  auto expr = ParseUnaryExpression();
  while (Match({MUL, DIV, MOD})) {
    Token op = Previous();
    auto right = ParseUnaryExpression();
    std::string type = actions_.CheckBinaryOperatorType(expr, right, op.type_);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

auto BaseParser::ParseAdditiveExpression() -> ExprPtr {
  auto expr = ParseMultiplicativeExpression();
  while (Match({ADD, SUB})) {
    Token op = Previous();
    auto right = ParseMultiplicativeExpression();
    std::string type = actions_.CheckBinaryOperatorType(expr, right, op.type_);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

auto BaseParser::ParseShiftExpression() -> ExprPtr {
  auto expr = ParseAdditiveExpression();
  while (Match({LEFT_OP, RIGHT_OP})) {
    Token op = Previous();
    auto right = ParseAdditiveExpression();
    std::string type = actions_.CheckShiftOperatorType(expr, right, op.type_);
    if (type.empty()) {
      ThrowParserException(
          makeString("invalid operands to binary expression ('{}' and '{}')",
                     expr->GetType(), right->GetType()));
    }
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

auto BaseParser::ParseRelationalExpression() -> ExprPtr {
  auto expr = ParseShiftExpression();
  while (Match({LE_OP, GE_OP, LT, GT})) {
    Token op = Previous();
    auto right = ParseShiftExpression();
    std::string type = actions_.CheckBinaryOperatorType(expr, right, op.type_);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

auto BaseParser::ParseEqualityExpression() -> ExprPtr {
  auto expr = ParseRelationalExpression();
  while (Match({EQ_OP, NE_OP})) {
    Token op = Previous();
    auto right = ParseRelationalExpression();
    std::string type = actions_.CheckBinaryOperatorType(expr, right, op.type_);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

auto BaseParser::ParseLogicalAndExpression() -> ExprPtr {
  auto expr = ParseEqualityExpression();
  while (Match(AND_OP)) {
    Token op = Previous();
    auto right = ParseEqualityExpression();
    std::string type = actions_.CheckBinaryOperatorType(expr, right, op.type_);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

auto BaseParser::ParseLogicalOrExpression() -> ExprPtr {
  auto expr = ParseLogicalAndExpression();
  while (Match(OR_OP)) {
    Token op = Previous();
    auto right = ParseLogicalAndExpression();
    std::string type = actions_.CheckBinaryOperatorType(expr, right, op.type_);
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

auto BaseParser::ParseAssignmentExpression() -> ExprPtr {
  auto expr = ParseLogicalOrExpression();
  if (Match(EQUAL)) {
    Token token = Previous();
    auto right = ParseAssignmentExpression();
    if (expr->GetType() != right->GetType()) {
      right =
          std::make_unique<ImplicitCastExpr>(expr->GetType(), std::move(right));
    }
    return std::make_unique<BinaryOperator>(token, std::move(expr),
                                            std::move(right), right->GetType());
  }
  return expr;
}

auto BaseParser::ParseExpression() -> ExprPtr {
  return ParseAssignmentExpression();
}

/**
 * parse Stmt
 */

auto BaseParser::ParseExpressionStatement() -> StmtPtr {
  ExprPtr expr = nullptr;
  if (!Match(SEMI)) {
    expr = ParseExpression();
    Consume(SEMI, "expected ';' after expression");
  }
  return std::make_unique<ExprStmt>(std::move(expr));
}

auto BaseParser::ParseReturnStatement() -> StmtPtr {
  Consume(RETURN, "expected 'return'");
  ExprPtr expr = nullptr;
  if (!Match(SEMI)) {
    expr = ParseExpression();
    Consume(SEMI, "expected ';' after expression");
  }
  return std::make_unique<ReturnStmt>(std::move(expr));
}

auto BaseParser::ParseIterationStatement() -> StmtPtr {
  if (Match(WHILE)) {
    Consume(LP, "expect '(' after 'while'");
    auto expr = ParseExpression();
    Consume(RP, "expect ')'");
    auto stmt = ParseStatement();
    return std::make_unique<WhileStmt>(std::move(expr), std::move(stmt));
  }
  if (Match(FOR)) {
    Consume(LP, "expect '(' after 'for'");
    StmtPtr init;
    if (Check({I64, F64})) {
      init = ParseDeclarationStatement();
    } else {
      ThrowParserException("not support expression statement now!");
    }
    auto cond = ParseExpression();
    Consume(SEMI, "expected ';' after expression");
    auto update = ParseExpression();
    Consume(RP, "expect ')'");
    auto body = ParseStatement();

    auto decl_stmt = dynamic_cast<DeclStmt *>(init.get());
    var_table_.erase(decl_stmt->decl_->GetName());
    return std::make_unique<ForStmt>(std::make_unique<DeclStmt>(decl_stmt),
                                     std::move(cond), std::move(update),
                                     std::move(body));
  }
  ThrowParserException("error in iteration statement");
  return nullptr;
}

auto BaseParser::ParseSelectionStatement() -> StmtPtr {
  if (Match(IF)) {
    Consume(LP, "expect '(' after 'if'");
    auto expr = ParseExpression();
    Consume(RP, "expect ')'");
    StmtPtr then_stmt;
    StmtPtr else_stmt;
    then_stmt = ParseStatement();
    if (Match(ELSE)) {
      else_stmt = ParseStatement();
    }
    return std::make_unique<IfStmt>(std::move(expr), std::move(then_stmt),
                                    std::move(else_stmt));
  }
  ThrowParserException("error in selection statement");
  return nullptr;
}

auto BaseParser::ParseDeclarationStatement() -> StmtPtr {
  Advance();
  auto type = Previous().value_;
  if (Match(IDENTIFIER)) {
    auto name = Previous().value_;
    auto decl = ParseVariableDeclaration(type, name, LOCAL);
    return std::make_unique<DeclStmt>(std::move(decl));
  }
  ThrowParserException("expected identifier");
  return nullptr;
}

auto BaseParser::ParseCompoundStatement() -> StmtPtr {
  Consume(LC, "expected function body after function declarator");
  std::vector<StmtPtr> stmts;
  while (!Check(RC) && current_.type_ != _EOF) {
    auto stmt = ParseStatement();
    stmts.push_back(std::move(stmt));
  }
  Consume(RC, "expected '}'");
  return std::make_unique<CompoundStmt>(std::move(stmts));
}

auto BaseParser::ParseStatement() -> StmtPtr {
  if (Check(RETURN)) {
    return ParseReturnStatement();
  }
  if (Check({VOID, I64, F64})) {
    return ParseDeclarationStatement();
  }
  if (Check(IF)) {
    return ParseSelectionStatement();
  }
  /// support while and for statement
  if (Check({WHILE, FOR})) {
    return ParseIterationStatement();
  }
  /// for other use of `parseCompoundStatement()`,
  /// use `check()` instead of `Match()`
  if (Check(LC)) {
    return ParseCompoundStatement();
  }
  return ParseExpressionStatement();
}

/**
 * internal parse
 */

auto BaseParser::ParseDeclarationSpecifiers() -> std::pair<std::string, bool> {
  std::string spec;
  bool is_extern = false;
  if (Match(EXTERN)) {
    is_extern = true;
  }
  if (Match({VOID, I64, F64})) {
    spec = Previous().value_;
  };
  return {spec, is_extern};
  ThrowParserException("expected type specifier");
  return {"", false};
}

auto BaseParser::ParseDeclarator() -> std::string {
  if (Match(IDENTIFIER)) {
    return Previous().value_;
  }
  ThrowParserException("expected identifier");
  return "";
}

auto BaseParser::ParseFunctionParameters()
    -> std::vector<std::unique_ptr<ParmVarDecl>> {
  std::vector<std::unique_ptr<ParmVarDecl>> params;
  if (!Check(RP)) {
    do {
      if (params.size() >= 255) {
        ThrowParserException("can't have more than 255 parameters");
      }
      auto [type, flag] = ParseDeclarationSpecifiers();
      auto name = ParseDeclarator();
      var_table_[name] = type;
      params.push_back(
          std::make_unique<ParmVarDecl>(std::move(name), std::move(type)));

    } while (Match(COMMA));
  }
  return params;
}

auto BaseParser::GenFuncType(std::string &&retTy,
                             std::vector<std::unique_ptr<ParmVarDecl>> &params)
    -> std::string {
  std::string func_type = retTy + " (";
  for (size_t i = 0; i < params.size(); i++) {
    func_type += params[i]->GetType();
    if (i != params.size() - 1) {
      func_type += ", ";
    }
  }
  func_type += ")";
  return func_type;
}

/**
 * parse Decl
 */

auto BaseParser::ParseVariableDeclaration(std::string type, std::string name,
                                          VarScope scope) -> DeclPtr {
  ExprPtr init;
  if (scope == GLOBAL) {
    if (global_var_table_.find(name) != global_var_table_.end()) {
      ThrowParserException(makeString("redefinition of '{}'", name));
    }
    /// for global variable, set default value
    ExprPtr zero;
    if (type == "i64") {
      zero = std::make_unique<IntegerLiteral>(0, "i64");
    } else if (type == "f64") {
      zero = std::make_unique<FloatingLiteral>(0, "f64");
    } else {
      ThrowParserException("not supported type");
    }
    /// TODO: support parse minus number
    init = (Match(EQUAL) ? ParseAssignmentExpression() : std::move(zero));
    if (!init->IsConstant()) {
      ThrowParserException(
          "initializer element is not a compile-time constant");
    }
    global_var_table_[name] = type;
  } else {
    if (var_table_.find(name) != var_table_.end()) {
      ThrowParserException(makeString("redefinition of '{}'", name));
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

auto BaseParser::ParseFunctionDeclaration(std::string ret_ty, std::string name,
                                          bool is_extern) -> DeclPtr {
  ClearVarTable();
  std::vector<std::unique_ptr<ParmVarDecl>> params = ParseFunctionParameters();
  Consume(RP, "expected parameter declarator");
  std::string func_type = GenFuncType(std::move(ret_ty), params);

  /// store in funcTable
  std::vector<std::string> params_ty;
  params_ty.reserve(params.size());
  for (auto &param : params) {
    params_ty.push_back(param->GetType());
  }
  FunctionParams fp(name, ret_ty, params_ty);
  func_table_[name] = fp;

  StmtPtr body = nullptr;
  /// if Match SEMI, body in null, otherwise, parse the function body
  if (!Match(SEMI)) {
    body = ParseCompoundStatement();
  }
  FuncKind kind =
      (is_extern ? EXTERN_FUNC : (body == nullptr ? DECLARATION : DEFINITION));
  return std::make_unique<FunctionDecl>(
      std::make_unique<FunctionProto>(std::move(name), std::move(func_type),
                                      std::move(params), 0),
      std::move(body), kind);
}

auto BaseParser::ParseExternalDeclaration() -> DeclPtr {
  auto [type, flag] = ParseDeclarationSpecifiers();
  auto name = ParseDeclarator();
  if (Match(LP)) {
    return ParseFunctionDeclaration(type, name, flag);
  }
  return ParseVariableDeclaration(type, name, GLOBAL);
}

/**
 * Parser
 */

auto Parser::Parse() -> std::unique_ptr<TranslationUnitDecl> {
  std::vector<DeclPtr> decls;
  Advance();
  while (current_.type_ != _EOF) {
    auto decl = ParseExternalDeclaration();
    decls.push_back(std::move(decl));
  }
  return std::make_unique<TranslationUnitDecl>(std::move(decls));
}

} // namespace toyc
