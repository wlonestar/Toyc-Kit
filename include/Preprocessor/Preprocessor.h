//! Toyc Preprocessor

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#pragma once

#include <Util.h>

namespace toyc {

class PreprocessorException : public std::exception {
private:
  size_t line;
  size_t col;
  std::string message;

public:
  PreprocessorException(size_t _line, size_t _col, std::string _msg)
      : line(_line), col(_col),
        message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                     "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                     _line, _col, _msg)) {}

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
  char peek();
  char peekNext();
  char advance();
  void backward();

private:
  /**
   * @brief Scan and remove single line comment
   *
   */
  void removeLineComment();

  /**
   * @brief Scan and remove multi line comment
   *
   */
  void removeMutliComment();

  /**
   * @brief replace `#include` macro with its file content
   *
   */
  void importLib();

public:
  Preprocessor() : input(""), start(0), current(0), line(1), col(0) {}

public:
  /**
   * @brief Set the `input`
   *
   * @param _input
   */
  void setInput(std::string _input);

  /**
   * @brief Preprocessor main method
   *
   * 1. replace `#include` macro with its content
   * 2. remove comments
   *
   * @notice: Lexer can not get correct error line now
   *
   * @return processed content fom `input`
   */
  std::string process();
};

} // namespace toyc

#endif
