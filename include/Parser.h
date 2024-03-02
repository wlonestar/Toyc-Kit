//! parser of toyc

#ifndef PARSER_H
#define PARSER_H

#pragma once

#include <AST.h>
#include <Lexer.h>
#include <Token.h>
#include <Util.h>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>

#include <cstddef>
#include <exception>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace toyc {

class ParserException : public std::exception {
private:
  size_t line;
  size_t col;
  std::string message;

public:
  ParserException(size_t _line, size_t _col, std::string &_message)
      : line(_line), col(_col),
        message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                     "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                     _line, _col, _message)) {}

  ParserException(size_t _line, size_t _col, std::string &&_message)
      : line(_line), col(_col),
        message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                     "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                     _line, _col, _message)) {}

  const char *what() const noexcept override { return message.c_str(); }
};

class Parser {
private:
  Token current;
  Token prev;
  Lexer lexer;
  /// global variable: <name, type>
  std::map<std::string, std::string> globalVarTable;
  /// local variable: <name, type>
  std::map<std::string, std::string> varTable;
  /// function declaration: <name, pair<retType, [type]...>>
  std::map<std::string, std::pair<std::string, std::vector<std::string>>>
      funcTable;

private:
  void throwParserException(std::string &&message) {
    throw ParserException(current.line, current.col, std::move(message));
  }
  void throwParserException(std::string &message) {
    throw ParserException(current.line, current.col, message);
  }

private:
  void clearVarTable() { varTable.clear(); }

public:
  Token peek() { return current; }
  Token previous() { return prev; }
  Token advance() {
    prev = current;
    for (;;) {
      current = lexer.scanToken();
      // debug("prev={}, curr={}", prev.toString(), current.toString());
      if (current.type != ERROR) {
        break;
      }
      throwParserException("error at parse");
    }
    return prev;
  }

  Token consume(TokenType type, std::string &message) {
    if (current.type == type) {
      return advance();
    }
    throwParserException(message);
    return Token(ERROR, "");
  }
  Token consume(TokenType type, std::string &&message) {
    if (current.type == type) {
      return advance();
    }
    throwParserException(message);
    return Token(ERROR, "");
  }

private:
  bool check(TokenType type) {
    if (type == _EOF) {
      return false;
    }
    return peek().type == type;
  }

  bool match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
      if (check(type)) {
        advance();
        return true;
      }
    }
    return false;
  }
  bool match(TokenType type) {
    if (check(type)) {
      advance();
      return true;
    }
    return false;
  }

private:
  bool checkHexadecimal(std::string &value) {
    if (value.starts_with("0x") || value.starts_with("0X")) {
      return true;
    }
    return false;
  }
  bool checkOctal(std::string &value) {
    if (value.starts_with("0") && value.size() != 1) {
      return true;
    }
    return false;
  }

  int64_t parseIntegerSuffix(std::string &value, int base);
  double parseFloatingSuffix(std::string &value, int base);

private:
  std::string checkUnaryOperatorType(TokenType type, Expr *right) {
    if (type == NOT) {
      return "i64";
    }
    return right->getType();
  }

  std::string checkBinaryOperatorType(TokenType type,
                                      std::unique_ptr<Expr> &left,
                                      std::unique_ptr<Expr> &right) {
    if (type == AND_OP || type == OR_OP) {
      return "i64";
    }
    if (left->getType() == right->getType()) {
      left =
          std::make_unique<ImplicitCastExpr>(left->getType(), std::move(left));
      right = std::make_unique<ImplicitCastExpr>(right->getType(),
                                                 std::move(right));
      return left->getType();
    }
    if (left->getType() == "f64" || right->getType() == "f64") {
      left = std::make_unique<ImplicitCastExpr>("f64", std::move(left));
      right = std::make_unique<ImplicitCastExpr>("f64", std::move(right));
      return "f64";
    }
    return "i64";
  }

private:
  std::unique_ptr<Expr> parseIntegerLiteral();
  std::unique_ptr<Expr> parseFloatingLiteral();

private:
  std::unique_ptr<Expr> parsePrimaryExpression();
  std::unique_ptr<Expr> parsePostfixExpression();
  std::unique_ptr<Expr> parseUnaryExpression();
  std::unique_ptr<Expr> parseMultiplicativeExpression();
  std::unique_ptr<Expr> parseAdditiveExpression();
  std::unique_ptr<Expr> parseRelationalExpression();
  std::unique_ptr<Expr> parseEqualityExpression();
  std::unique_ptr<Expr> parseLogicalAndExpression();
  std::unique_ptr<Expr> parseLogicalOrExpression();
  std::unique_ptr<Expr> parseAssignmentExpression();
  std::unique_ptr<Expr> parseExpression();

private:
  std::unique_ptr<Stmt> parseExpressionStatement();
  std::unique_ptr<Stmt> parseReturnStatement();
  std::unique_ptr<Stmt> parseDeclarationStatement();
  std::unique_ptr<Stmt> parseCompoundStatement();
  std::unique_ptr<Stmt> parseStatement();

private:
  std::pair<std::string, bool> parseDeclarationSpecifiers();
  std::string parseDeclarator();
  std::vector<std::unique_ptr<ParmVarDecl>> parseFunctionParameters();
  std::string
  generateFunctionType(std::string &&retType,
                       std::vector<std::unique_ptr<ParmVarDecl>> &params);

private:
  std::unique_ptr<Decl> parseVariableDeclaration(std::string &&type,
                                                 std::string &&name,
                                                 VarScope scope);
  std::unique_ptr<Decl> parseFunctionDeclaration(std::string &&type,
                                                 std::string &&name,
                                                 bool isExtern);
  std::unique_ptr<Decl> parseExternalDeclaration();

public:
  Parser() : lexer(), current(), prev() {}

  void addInput(std::string &_input) { lexer.addInput(_input); }
  std::string getInput() { return lexer.getInput(); }

  std::unique_ptr<TranslationUnitDecl> parse();
};

} // namespace toyc

#endif
