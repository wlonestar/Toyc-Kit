//! parser of toyc

#ifndef PARSER_H
#define PARSER_H

#include <llvm-16/llvm/IR/Instructions.h>
#include <llvm-16/llvm/IR/Value.h>
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
#include <vector>

namespace toyc {

/// <Name, pair<Type, Value *>>
extern std::map<std::string, std::pair<std::string, llvm::Value *>>
    VariableTable;

/// <Name, tuple<Type, Alloca *, Value *>>
extern std::map<std::string,
                std::tuple<std::string, llvm::AllocaInst *, llvm::Value *>>
    LocalVariableTable;

/// <Name, pair<Type, Function *>>
extern std::map<std::string, std::pair<std::string, llvm::Function *>>
    FunctionTable;

void printVariableTable();
void printLocalVariableTable();

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
      debug("prev={}, curr={}", prev.toString(), current.toString());
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

  int64_t parseIntegerSuffix(std::string &value, int base);
  double parseFloatingSuffix(std::string &value, int base);

private:
  std::string checkUnaryOperatorType(TokenType type, Expr *right) {
    if (type == NOT) {
      return "i64";
    }
    return right->getType();
  }

  std::string checkBinaryOperatorType(TokenType type, Expr *left, Expr *right) {
    if (type == AND_OP || type == OR_OP) {
      return "i64";
    }
    if (left->getType() == right->getType()) {
      return left->getType();
    }
    if (left->getType() == "f64" || right->getType() == "f64") {
      return "f64";
    }
    return "i64";
  }

private:
  std::unique_ptr<Expr> parseIntegerLiteral();
  std::unique_ptr<Expr> parseFloatingLiteral();

private:
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
  std::unique_ptr<Stmt> parseExpressionStatement();
  std::unique_ptr<Stmt> parseReturnStatement();
  std::unique_ptr<Stmt> parseDeclarationStatement();
  std::unique_ptr<Stmt> parseCompoundStatement();
  std::unique_ptr<Stmt> parseStatement();

private:
  std::string parseDeclarationSpecifiers();
  std::string parseDeclarator();

private:
  std::unique_ptr<Decl> parseVariableDeclaration(std::string &&type,
                                                 std::string &&name,
                                                 VarScope scope);
  std::unique_ptr<Decl> parseFunctionDeclaration(std::string &&type,
                                                 std::string &&name);
  std::unique_ptr<Decl> parseExternalDeclaration();

public:
  Parser() : lexer(), current(), prev() {}

  void addInput(std::string &_input) { lexer.addInput(_input); }
  std::string getInput() { return lexer.getInput(); }

  std::unique_ptr<TranslationUnitDecl> parse();
};

} // namespace toyc

#endif
