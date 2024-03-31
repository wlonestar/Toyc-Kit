//! parser of toyc

#ifndef PARSER_H
#define PARSER_H

#pragma once

#include <AST/AST.h>
#include <Lexer/Lexer.h>
#include <Sema/Sema.h>

#include <cstddef>
#include <exception>
#include <initializer_list>
#include <tuple>
#include <vector>

namespace toyc {

class ParserException : public std::exception {
private:
  size_t line_;
  size_t col_;
  std::string message_;

public:
  ParserException(size_t _line, size_t _col, std::string _msg)
      : line_(_line), col_(_col),
        message_(makeString("\033[1;37mline:{}:col:{}:\033[0m "
                            "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                            _line, _col, _msg)) {}

  auto what() const noexcept -> const char * override {
    return message_.c_str();
  }
};

struct FunctionParams {
  std::string name_;
  std::string ret_type_;
  std::vector<std::string> params_;

  FunctionParams() = default;

  FunctionParams(std::string _name, std::string _retType,
                 std::vector<std::string> &_params)
      : name_(std::move(_name)), ret_type_(std::move(_retType)),
        params_(_params) {}
};

class BaseParser {
protected:
  Token current_;
  Token prev_;
  Lexer lexer_;
  Sema actions_;

protected:
  /// global variable: <name, type>
  std::map<std::string, std::string> global_var_table_;
  /// local variable: <name, type>
  std::map<std::string, std::string> var_table_;
  /// function declaration: <name, pair<retType, [type]...>>
  std::map<std::string, FunctionParams> func_table_;

protected:
  void ThrowParserException(const std::string &message) {
    throw ParserException(current_.line_, current_.col_, message);
  }

protected:
  void ClearVarTable() { var_table_.clear(); }

public:
  auto Peek() -> Token { return current_; }
  auto Previous() -> Token { return prev_; }
  auto Advance() -> Token;
  auto Consume(TokenTy type, std::string &message) -> Token;
  auto Consume(TokenTy type, std::string &&message) -> Token;

protected:
  auto Check(std::initializer_list<TokenTy> types) -> bool;
  auto Check(TokenTy type) -> bool;
  auto Match(std::initializer_list<TokenTy> types) -> bool;
  auto Match(TokenTy type) -> bool;

protected:
  auto ParseIntegerSuffix(std::string &value, int base) -> int64_t;
  auto ParseFloatingSuffix(std::string &value, int base) -> double;

protected:
  auto ParseIntegerLiteral() -> ExprPtr;
  auto ParseFloatingLiteral() -> ExprPtr;
  auto ParsePrimaryExpression() -> ExprPtr;
  auto ParsePostfixExpression() -> ExprPtr;
  auto ParseUnaryExpression() -> ExprPtr;
  auto ParseMultiplicativeExpression() -> ExprPtr;
  auto ParseAdditiveExpression() -> ExprPtr;
  auto ParseShiftExpression() -> ExprPtr;
  auto ParseRelationalExpression() -> ExprPtr;
  auto ParseEqualityExpression() -> ExprPtr;
  auto ParseLogicalAndExpression() -> ExprPtr;
  auto ParseLogicalOrExpression() -> ExprPtr;
  auto ParseAssignmentExpression() -> ExprPtr;
  auto ParseExpression() -> ExprPtr;

public:
  auto ParseExpressionStatement() -> StmtPtr;
  auto ParseReturnStatement() -> StmtPtr;
  auto ParseIterationStatement() -> StmtPtr;
  auto ParseSelectionStatement() -> StmtPtr;
  auto ParseDeclarationStatement() -> StmtPtr;
  auto ParseCompoundStatement() -> StmtPtr;
  auto ParseStatement() -> StmtPtr;

public:
  auto ParseDeclarationSpecifiers() -> std::pair<std::string, bool>;
  auto ParseDeclarator() -> std::string;
  auto ParseFunctionParameters() -> std::vector<std::unique_ptr<ParmVarDecl>>;
  auto GenFuncType(std::string &&retTy,
                   std::vector<std::unique_ptr<ParmVarDecl>> &params)
      -> std::string;

public:
  virtual auto ParseVariableDeclaration(std::string type, std::string name,
                                        VarScope scope) -> DeclPtr;
  auto ParseFunctionDeclaration(std::string ret_type, std::string name,
                                bool is_extern) -> DeclPtr;

  auto ParseExternalDeclaration() -> DeclPtr;

public:
  BaseParser() = default;

public:
  void AddInput(std::string &_input) { lexer_.AddInput(_input); }
  auto GetInput() -> std::string { return lexer_.GetInput(); }
};

class Parser : public BaseParser {
public:
  Parser() = default;

public:
  auto Parse() -> std::unique_ptr<TranslationUnitDecl>;
};

} // namespace toyc

#endif
