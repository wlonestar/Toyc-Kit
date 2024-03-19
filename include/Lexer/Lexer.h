//! Toyc lexical analyzer

#ifndef LEXER_H
#define LEXER_H

#pragma once

#include <Lexer/Token.h>

#include <exception>

namespace toyc {

class LexerException : public std::exception {
private:
  size_t line;
  size_t col;
  std::string message;

public:
  LexerException(size_t _line, size_t _col, std::string _msg)
      : line(_line), col(_col),
        message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                     "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                     _line, _col, _msg)) {}

  const char *what() const noexcept override { return message.c_str(); }
};

enum LexerExceptionCode {
  INVALID_INTEGER_SUFFIX = 0,
  INVALID_FLOATING_SUFFIX = 1,
  INVALID_INTEGER_OR_FLOATING = 2,
};

static std::vector<std::string> LexerExceptionTable = {
    "invalid suffix on integer constant",   // INVALID_INTEGER_SUFFIX
    "invalid suffix on floating constant",  // INVALID_FLOATING_SUFFIX
    "invalid integer or floating constant", // INVALID_INTEGER_OR_FLOATING
};

/**
 * @brief Toyc lexical analyzer
 *
 * Support Incremental lexical analysis
 *
 */
class Lexer {
private:
  /// String that need to be scan
  std::string input;
  /// Cursor of input that point to begin character of current token
  size_t start;
  /// Cursor of input that points to current character of current token
  size_t current;
  /// Line number of the input, begin from 1
  size_t line;
  /// Column number of current line, begin from 0
  size_t col;

private:
  void throwLexerException(std::string message) {
    throw LexerException(line, col, message);
  }

private:
  bool isEnd() { return current >= input.size(); }
  bool isHexPreifx(char a, char b) {
    return a == '0' && (b == 'x' || b == 'X');
  }
  bool isCharPrefix(char c) { return c == 'u' || c == 'U' || c == 'L'; }

private:
  /**
   * @brief Check current character if match with `expected`
   *
   * @param expected
   */
  bool match(char expected);

  /**
   * @brief Move `current` cursor forward `steps`
   *
   * @param steps
   */
  void forward(size_t steps);

  /**
   * @brief Move `current` cursor backward `steps`
   *
   * @param steps
   */
  void backward(size_t steps);

public:
  /**
   * @brief Move `current` cursor forward one step and return current character
   *
   * @return char
   */
  char advance();

  /**
   * @brief Return character that located at `current` cursor
   *
   * @return char
   */
  char peek();

  /**
   * @brief Return character that next to `current` cursor
   *
   * @return char
   */
  char peekNext();

  /**
   * @brief Return character that before `current` cursor
   *
   * @return char
   */
  char previous();

private:
  /**
   * @brief Return token that depends on `start` and `current` cursor
   *
   * @param type
   * @return Token
   */
  Token makeToken(TokenTy type);

  /**
   * @brief Return token that depends on customed `value`
   *
   * @param type
   * @param value
   * @return Token
   */
  Token makeToken(TokenTy type, std::string value);

  /**
   * @brief Skip whitespace
   *
   */
  void skipWhitespace();

  /**
   * @brief Skip multi line commet
   *
   */
  void skipMutliComment();

private:
  /**
   * @brief Scan string literal, remove quotation marks from both sides of the
   * string
   *
   * @return Token
   */
  Token scanString();

  /**
   * @brief Scan number literal, use regex to simplify code
   *
   * @return Token
   */
  Token scanNumber();

  /**
   * @brief Scan identifier, search `KeywordTable` to match keyword
   *
   * @return Token
   */
  Token scanIdentifier();

public:
  Lexer() : input(""), start(0), current(0), line(1), col(0) {}

public:
  /**
   * @brief Append `str` behind `input`, reset line and column count
   *
   * @param str
   */
  void addInput(std::string str);

  /**
   * @brief Get the `input` string
   *
   * @return std::string
   */
  std::string getInput();

public:
  /**
   * @brief Scan and return a Token
   *
   * @return Token
   */
  Token scanToken();
};

} // namespace toyc

#endif
