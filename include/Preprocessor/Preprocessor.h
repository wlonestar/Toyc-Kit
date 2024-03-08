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
  PreprocessorException(size_t _line, size_t _col, std::string &_message)
      : line(_line), col(_col),
        message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                     "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                     _line, _col, _message)) {}

  PreprocessorException(size_t _line, size_t _col, std::string &&_message)
      : line(_line), col(_col),
        message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                     "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                     _line, _col, _message)) {}

  const char *what() const noexcept override { return message.c_str(); }
};

class Preprocessor {
private:
  std::string input;
  size_t start;
  size_t current;
  size_t line;
  size_t col;
  char *arg0;

private:
  void throwPreprocessorException(std::string &&message) {
    throw PreprocessorException(line, col, std::move(message));
  }
  void throwPreprocessorException(std::string &message) {
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

  void skipMutliComment();

  bool readFrom(std::string &src, std::string &input);
  void importLib();

public:
  Preprocessor(char *_arg0)
      : input(""), start(0), current(0), line(1), col(0), arg0(_arg0) {}

  void addInput(std::string &_input);

  std::string process();
};

} // namespace toyc

#endif
