//! preprocessor

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#pragma once

#include <Util.h>

#include <string>

namespace toyc {

class PreprocessorException : public std::exception {
private:
  size_t line;
  size_t col;
  std::string message;

public:
  PreprocessorException(size_t _line, size_t _col, std::string _message)
      : line(_line), col(_col),
        message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                     "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                     _line, _col, _message)) {}

  const char *what() const noexcept override { return message.c_str(); }
};

/**
 * @brief Toyc Preprocessor
 *
 * 1. remove comments
 * 2. replace `#include` macros with its contents
 */
class Preprocessor {
private:
  std::string input;
  size_t start;
  size_t current;
  size_t line;
  size_t col;

private:
  void throwPreprocessorException(std::string message) {
    throw PreprocessorException(line, col, message);
  }

private:
  bool isEnd();
  bool isD(char c) { return (c >= '0' && c <= '9'); }
  bool isL(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }
  bool isA(char c) { return isL(c) || isD(c); }

  char peek();
  char peekNext();
  char advance();
  void backward();

private:
  void skipLineComment();
  void skipMutliComment();
  void importLib();

public:
  Preprocessor() : input(""), start(0), current(0), line(1), col(0) {}

  void setInput(std::string &_input) { input = _input; }
  void setInput(std::string &&_input) { input = _input; }

  /**
   * @brief Preprocessor main method
   *
   * 1. replace `#include` macro with its content
   * 2. remove comments
   *
   * @notice: Lexer can not get correct error message line now
   *
   * @return processed content fom `input`
   */
  std::string process();
};

} // namespace toyc

#endif
