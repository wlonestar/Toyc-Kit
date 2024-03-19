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
  size_t line;
  size_t col;
  std::string message;

public:
  ParserException(size_t _line, size_t _col, std::string _msg)
      : line(_line), col(_col),
        message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                     "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                     _line, _col, _msg)) {}

  const char *what() const noexcept override { return message.c_str(); }
};

struct FunctionParams {
  std::string name;
  std::string retType;
  std::vector<std::string> params;

  FunctionParams() {}

  FunctionParams(std::string _name, std::string _retType,
                 std::vector<std::string> &_params)
      : name(_name), retType(_retType), params(_params) {}
};

class BaseParser {
protected:
  Token current;
  Token prev;
  Lexer lexer;
  Sema actions;

protected:
  /// global variable: <name, type>
  std::map<std::string, std::string> globalVarTable;
  /// local variable: <name, type>
  std::map<std::string, std::string> varTable;
  /// function declaration: <name, pair<retType, [type]...>>
  std::map<std::string, FunctionParams> funcTable;

protected:
  void throwParserException(std::string message) {
    throw ParserException(current.line, current.col, std::move(message));
  }

protected:
  void clearVarTable() { varTable.clear(); }

public:
  Token peek() { return current; }
  Token previous() { return prev; }
  Token advance();
  Token consume(TokenTy type, std::string &message);
  Token consume(TokenTy type, std::string &&message);

protected:
  bool check(std::initializer_list<TokenTy> types);
  bool check(TokenTy type);
  bool match(std::initializer_list<TokenTy> types);
  bool match(TokenTy type);

protected:
  int64_t parseIntegerSuffix(std::string &value, int base);
  double parseFloatingSuffix(std::string &value, int base);

protected:
  ExprPtr parseIntegerLiteral();
  ExprPtr parseFloatingLiteral();
  ExprPtr parsePrimaryExpression();
  ExprPtr parsePostfixExpression();
  ExprPtr parseUnaryExpression();
  ExprPtr parseMultiplicativeExpression();
  ExprPtr parseAdditiveExpression();
  ExprPtr parseShiftExpression();
  ExprPtr parseRelationalExpression();
  ExprPtr parseEqualityExpression();
  ExprPtr parseLogicalAndExpression();
  ExprPtr parseLogicalOrExpression();
  ExprPtr parseAssignmentExpression();
  ExprPtr parseExpression();

public:
  StmtPtr parseExpressionStatement();
  StmtPtr parseReturnStatement();
  StmtPtr parseIterationStatement();
  StmtPtr parseSelectionStatement();
  StmtPtr parseDeclarationStatement();
  StmtPtr parseCompoundStatement();
  StmtPtr parseStatement();

public:
  std::pair<std::string, bool> parseDeclarationSpecifiers();
  std::string parseDeclarator();
  std::vector<std::unique_ptr<ParmVarDecl>> parseFunctionParameters();
  std::string genFuncType(std::string &&retTy,
                          std::vector<std::unique_ptr<ParmVarDecl>> &params);

public:
  virtual DeclPtr parseVariableDeclaration(std::string type, std::string name,
                                           VarScope scope);
  DeclPtr parseFunctionDeclaration(std::string type, std::string name,
                                   bool isExtern);

  DeclPtr parseExternalDeclaration();

public:
  BaseParser() : lexer(), current(), prev(), actions() {}

public:
  void addInput(std::string &_input) { lexer.addInput(_input); }
  std::string getInput() { return lexer.getInput(); }
};

class Parser : public BaseParser {
public:
  Parser() : BaseParser() {}

public:
  std::unique_ptr<TranslationUnitDecl> parse();
};

} // namespace toyc

#endif
