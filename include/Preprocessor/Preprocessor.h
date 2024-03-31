//! Toyc Preprocessor

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#pragma once

#include <Util.h>

namespace toyc {

class PreprocessorException : public std::exception {
private:
  size_t line_;
  size_t col_;
  std::string message_;

public:
  PreprocessorException(size_t _line, size_t _col, std::string _msg)
      : line_(_line), col_(_col),
        message_(makeString("\033[1;37mline:{}:col:{}:\033[0m "
                            "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                            _line, _col, _msg)) {}

  auto what() const noexcept -> const char * override {
    return message_.c_str();
  }
};

/**
 * @brief Toyc Preprocessor
 *
 * 1. remove comments
 * 2. replace `#include` macros with its contents
 */
class Preprocessor {
private:
  std::string input_;
  size_t start_{};
  size_t current_{};
  size_t line_{};
  size_t col_{};

private:
  void ThrowPreprocessorException(std::string message) {
    throw PreprocessorException(line_, col_, std::move(message));
  }

private:
  auto IsEnd() -> bool;
  auto Peek() -> char;
  auto PeekNext() -> char;
  auto Advance() -> char;
  void Backward();

private:
  /**
   * @brief Scan and remove single line comment
   *
   */
  void RemoveLineComment();

  /**
   * @brief Scan and remove multi line comment
   *
   */
  void RemoveMutliComment();

  /**
   * @brief replace `#include` macro with its file content
   *
   */
  void ImportLib();

public:
  Preprocessor() = default;

public:
  /**
   * @brief Set the `input`
   *
   * @param _input
   */
  void SetInput(std::string _input);

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
  auto Process() -> std::string;
};

} // namespace toyc

#endif
