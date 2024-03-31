//! Toyc lexical analyzer

#ifndef LEXER_H
#define LEXER_H

#pragma once

#include <Lexer/Token.h>

#include <exception>

namespace toyc {

class LexerException : public std::exception {
private:
  size_t line_;
  size_t col_;
  std::string message_;

public:
  LexerException(size_t _line, size_t _col, std::string _msg)
      : line_(_line), col_(_col),
        message_(makeString("\033[1;37mline:{}:col:{}:\033[0m "
                            "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                            _line, _col, _msg)) {}

  auto what() const noexcept -> const char * override {
    return message_.c_str();
  }
};

enum LexerExceptionCode {
  INVALID_INTEGER_SUFFIX = 0,
  INVALID_FLOATING_SUFFIX = 1,
  INVALID_INTEGER_OR_FLOATING = 2,
};

static std::vector<std::string> lexer_exception_table = {
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
  std::string input_;
  /// Cursor of input that point to begin character of current token
  size_t start_{};
  /// Cursor of input that points to current character of current token
  size_t current_{};
  /// Line number of the input, begin from 1
  size_t line_{};
  /// Column number of current line, begin from 0
  size_t col_{};

private:
  void ThrowLexerException(std::string message) {
    throw LexerException(line_, col_, std::move(message));
  }

private:
  auto IsEnd() -> bool { return current_ >= input_.size(); }
  auto IsHexPreifx(char a, char b) -> bool {
    return a == '0' && (b == 'x' || b == 'X');
  }
  auto IsCharPrefix(char c) -> bool { return c == 'u' || c == 'U' || c == 'L'; }

private:
  /**
   * @brief Check current character if match with `expected`
   *
   * @param expected
   */
  auto Match(char expected) -> bool;

  /**
   * @brief Move `current` cursor forward `steps`
   *
   * @param steps
   */
  void Forward(size_t steps);

  /**
   * @brief Move `current` cursor backward `steps`
   *
   * @param steps
   */
  void Backward(size_t steps);

public:
  /**
   * @brief Move `current` cursor forward one step and return current character
   *
   * @return char
   */
  auto Advance() -> char;

  /**
   * @brief Return character that located at `current` cursor
   *
   * @return char
   */
  auto Peek() -> char;

  /**
   * @brief Return character that next to `current` cursor
   *
   * @return char
   */
  auto PeekNext() -> char;

  /**
   * @brief Return character that before `current` cursor
   *
   * @return char
   */
  auto Previous() -> char;

private:
  /**
   * @brief Return token that depends on `start` and `current` cursor
   *
   * @param type
   * @return Token
   */
  auto MakeToken(TokenTy type) -> Token;

  /**
   * @brief Return token that depends on customed `value`
   *
   * @param type
   * @param value
   * @return Token
   */
  auto MakeToken(TokenTy type, std::string value) -> Token;

  /**
   * @brief Skip whitespace
   *
   */
  void SkipWhitespace();

  /**
   * @brief Skip multi line commet
   *
   */
  void SkipMutliComment();

private:
  /**
   * @brief Scan string literal, remove quotation marks from both sides of the
   * string
   *
   * @return Token
   */
  auto ScanString() -> Token;

  /**
   * @brief Scan number literal, use regex to simplify code
   *
   * @return Token
   */
  auto ScanNumber() -> Token;

  /**
   * @brief Scan identifier, search `KeywordTable` to match keyword
   *
   * @return Token
   */
  auto ScanIdentifier() -> Token;

public:
  Lexer() = default;

public:
  /**
   * @brief Append `str` behind `input`, reset line and column count
   *
   * @param str
   */
  void AddInput(const std::string &input);

  /**
   * @brief Get the `input` string
   *
   * @return std::string
   */
  auto GetInput() -> std::string;

public:
  /**
   * @brief Scan and return a Token
   *
   * @return Token
   */
  auto ScanToken() -> Token;
};

} // namespace toyc

#endif
