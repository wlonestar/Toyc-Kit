//! parser of toyc

#ifndef INTERPRETER_PARSER_H
#define INTERPRETER_PARSER_H

#pragma once

#include <AST/AST.h>
#include <Lexer/Lexer.h>
#include <Parser/Parser.h>
#include <Util.h>

#include <cstddef>
#include <exception>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace toyc {

class InterpreterParser {
private:
  Token current;
  Token prev;
  Lexer lexer;
  /// global variable: <name, type>
  std::map<std::string, std::string> globalVarTable;
  /// local variable: <name, type>
  std::map<std::string, std::string> varTable;
  /// function declaration: <name, pair<retType, [type]...>>
  std::map<std::string, FunctionParams> funcTable;

private:
  using parse_t = std::variant<std::unique_ptr<Decl>, std::unique_ptr<Stmt>,
                               std::unique_ptr<Expr>>;

protected:
  void throwInterParserException(std::string &&msg) {
    throw ParserException(current.line, current.col, std::move(msg));
  }
  void throwInterParserException(std::string &msg) {
    throw ParserException(current.line, current.col, msg);
  }

protected:
  void clearVarTable() { varTable.clear(); }

public:
  Token peek() { return current; }
  Token previous() { return prev; }
  Token advance();
  Token consume(TokenType type, std::string &message);
  Token consume(TokenType type, std::string &&message);

protected:
  bool check(std::initializer_list<TokenType> types);
  bool check(TokenType type);
  bool match(std::initializer_list<TokenType> types);
  bool match(TokenType type);

protected:
  bool checkHexadecimal(std::string &value);
  bool checkOctal(std::string &value);

  int64_t parseIntegerSuffix(std::string &value, int base);
  double parseFloatingSuffix(std::string &value, int base);

  std::string checkUnaryOperatorType(TokenType type, Expr *right);
  std::string checkBinaryOperatorType(TokenType type,
                                      std::unique_ptr<Expr> &left,
                                      std::unique_ptr<Expr> &right);
  std::string checkShiftOperatorType(TokenType type,
                                     std::unique_ptr<Expr> &left,
                                     std::unique_ptr<Expr> &right);

protected:
  std::unique_ptr<Expr> parseIntegerLiteral();
  std::unique_ptr<Expr> parseFloatingLiteral();

  std::unique_ptr<Expr> parseConstant();
  std::unique_ptr<Expr> parsePrimaryExpression();
  std::unique_ptr<Expr> parsePostfixExpression();
  std::unique_ptr<Expr> parseUnaryExpression();
  std::unique_ptr<Expr> parseCastExpression();
  std::unique_ptr<Expr> parseMultiplicativeExpression();
  std::unique_ptr<Expr> parseAdditiveExpression();
  std::unique_ptr<Expr> parseShiftExpression();
  std::unique_ptr<Expr> parseRelationalExpression();
  std::unique_ptr<Expr> parseEqualityExpression();
  std::unique_ptr<Expr> parseLogicalAndExpression();
  std::unique_ptr<Expr> parseLogicalOrExpression();
  std::unique_ptr<Expr> parseAssignmentExpression();
  std::unique_ptr<Expr> parseExpression();

  std::unique_ptr<Stmt> parseExpressionStatement();
  std::unique_ptr<Stmt> parseReturnStatement();
  std::unique_ptr<Stmt> parseIterationStatement();
  std::unique_ptr<Stmt> parseSelectionStatement();
  std::unique_ptr<Stmt> parseDeclarationStatement();
  std::unique_ptr<Stmt> parseCompoundStatement();
  std::unique_ptr<Stmt> parseStatement();

  std::pair<std::unique_ptr<ExprStmt>, bool>
  parseExpressionOrExpressionStatement();

  std::pair<std::string, bool> parseDeclarationSpecifiers();
  std::string parseDeclarator();
  std::vector<std::unique_ptr<ParmVarDecl>> parseFunctionParameters();
  std::string genFuncType(std::string &&retTy,
                          std::vector<std::unique_ptr<ParmVarDecl>> &params);

  std::unique_ptr<Decl> parseVariableDeclaration(std::string &type,
                                                 std::string &name,
                                                 VarScope scope);
  std::unique_ptr<Decl>
  parseFunctionDeclaration(std::string &type, std::string &name, bool isExtern);
  virtual std::unique_ptr<Decl> parseExternalDeclaration();

public:
  InterpreterParser() : lexer(), current(), prev() {}

  void addInput(std::string &_input) { lexer.addInput(_input); }
  std::string getInput() { return lexer.getInput(); }

  parse_t parse();
};

} // namespace toyc

#endif
