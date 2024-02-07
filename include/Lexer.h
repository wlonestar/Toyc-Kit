//! lexer of toyc

#ifndef LEXER_H
#define LEXER_H

#pragma once

#include <Token.h>
#include <Util.h>

#include <cstddef>
#include <exception>
#include <map>

namespace toyc {

class LexerError : public std::exception {
private:
  size_t line;
  size_t col;
  std::string message;

public:
  LexerError(size_t _line, size_t _col, std::string &_message);
  LexerError(size_t _line, size_t _col, std::string &&_message);

  const char *what() const noexcept override { return message.c_str(); }
};

enum LexerErrorCode {
  INVALID_INTEGER_SUFFIX,
  INVALID_FLOATING_SUFFIX,
  INVALID_INTEGER_OR_FLOATING,
};

static std::map<LexerErrorCode, std::string> LexerErrorTable = {
    {INVALID_INTEGER_SUFFIX, "invalid suffix on integer constant"},
    {INVALID_FLOATING_SUFFIX, "invalid suffix on floating constant"},
    {INVALID_INTEGER_OR_FLOATING, "invalid integer or floating constant"},
};

class Lexer {
private:
  std::string input;
  size_t start;
  size_t current;
  size_t line;
  size_t col;

private:
  void throwLexerError(std::string &&message) {
    throw LexerError(line, col, std::move(message));
  }
  void throwLexerError(std::string &message) {
    throw LexerError(line, col, message);
  }

private:
  bool isEnd() { return current >= input.size(); }
  bool match(char expected) {
    if (isEnd()) {
      return false;
    }
    if (input.at(current) != expected) {
      return false;
    }
    col++;
    current++;
    return true;
  }
  bool isD(char c) { return (c >= '0' && c <= '9'); }
  bool isNZ(char c) { return (c >= '1' && c <= '9'); }
  bool isL(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }
  bool isA(char c) { return isL(c) || isD(c); }
  bool isHP(char a, char b) { return a == '0' && (b == 'x' || b == 'X'); }
  bool isCP(char c) { return c == 'u' || c == 'U' || c == 'L'; }

private:
  void forward(size_t steps) {
    current += steps;
    col += steps;
  }
  void backward(size_t steps) {
    current -= steps;
    col -= steps;
  }

  char advance() {
    col++;
    return input.at(current++);
  }
  char peek() {
    if (isEnd()) {
      return '\0';
    }
    return input.at(current);
  }

public:
  char peekNext() {
    if (current + 1 >= input.size()) {
      return '\0';
    }
    return input.at(current + 1);
  }

private:
  char previous() {
    if (current - 1 < 0) {
      return '\0';
    }
    return input.at(current - 1);
  }

private:
  Token makeToken(TokenType type) {
    return Token(type, input.substr(start, current - start), line, col);
  }
  Token makeToken(TokenType type, std::string &&value) {
    return Token(type, std::move(value), line, col);
  }

private:
  void skipWhitespace();
  void skipMutliComment();

  Token scanString();
  Token scanNumber();
  Token scanIdentifier();

public:
  Lexer() : input(""), start(0), current(0), line(1), col(0) {}

  void addInput(std::string &_input) {
    if (input.size() == 0) {
      input = _input;
    } else {
      input = input + '\n' + _input;
      /// reset column cursor
      current++;
      line++;
      col = 0;
    }
  }

  std::string getInput() { return input; }

  Token scanToken();
};

} // namespace toyc

#endif
