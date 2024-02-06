//! parser of toyc

#ifndef PARSER_H
#define PARSER_H

#pragma once

#include <AST.h>
#include <Lexer.h>
#include <Token.h>
#include <Util.h>

#include <cstddef>
#include <exception>
#include <initializer_list>
#include <memory>
#include <tuple>

namespace toyc {

extern std::map<std::string, llvm::Value *> VariableTable;

class ParserError : public std::exception {
private:
  size_t line;
  size_t col;
  std::string message;

public:
  ParserError(size_t _line, size_t _col, std::string &_message);
  ParserError(size_t _line, size_t _col, std::string &&_message);

  const char *what() const noexcept override { return message.c_str(); }
};

class Parser {
private:
  Token current;
  Token prev;

  Lexer lexer;

private:
  void throwParserError(std::string &&message) {
    throw ParserError(current.line, current.col, std::move(message));
  }
  void throwParserError(std::string &message) {
    throw ParserError(current.line, current.col, message);
  }

public:
  Token peek() { return current; }
  Token previous() { return prev; }
  Token advance() {
    prev = current;
    for (;;) {
      current = lexer.scanToken();
      debug("previous={}, current={}", prev.toString(), current.toString());
      if (current.type != ERROR) {
        break;
      }
      throwParserError("error at parse");
    }
    return prev;
  }

  Token consume(TokenType type, std::string &message) {
    if (current.type == type) {
      return advance();
    }
    throwParserError(message);
    return Token(ERROR, "");
  }
  Token consume(TokenType type, std::string &&message) {
    if (current.type == type) {
      return advance();
    }
    throwParserError(message);
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

  std::tuple<std::string, integer_t> parseIntegerSuffix(std::string &value,
                                                        int base);
  std::tuple<std::string, floating_t> parseFloatingSuffix(std::string &value,
                                                          int base);

private:
  std::string checkBinaryOperatorType(TokenType type, Expr *left, Expr *right) {
    if (type == AND_OP || type == OR_OP) {
      return "int";
    }
    if (left->getType() == right->getType()) {
      return left->getType();
    }
    if (left->getType() == "double" || right->getType() == "double") {
      return "double";
    }
    return "int";
  }

private:
  std::unique_ptr<Expr> parseIntegerLiteral();
  std::unique_ptr<Expr> parseFloatingLiteral();

  std::unique_ptr<Expr> parsePrimaryExpression();

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
  std::unique_ptr<Decl> parseDeclaration();

public:
  Parser(std::string &input) : lexer(input) {}

  std::unique_ptr<Decl> parse();
};

} // namespace toyc

#endif
