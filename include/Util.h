#ifndef UTIL_H
#define UTIL_H

#pragma once

#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>

namespace toyc {

/**
 * @brief wrap a macro for providing a simple string formatter
 *
 */
// #define fstr(__fmt__, ...) fmt::format(__fmt__, ##__VA_ARGS__)

#define makeString(__fmt__, ...) std::move(fmt::format(__fmt__, ##__VA_ARGS__))

/**
 * @brief print info with located file and line number, for debugging
 *
 * @notice: only works on DEBUG mode
 */
#ifndef NDEBUG
#define debug(__fmt__, ...)                                                    \
  std::cerr << makeString("\033[1;34m{}:{} [debug] " __fmt__ "\033[0m\n",      \
                          __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(__fmt__, ...) ((void)0)
#endif

static bool isDigit(char c) { return (c >= '0' && c <= '9'); }
static bool isNonZero(char c) { return (c >= '1' && c <= '9'); }
static bool isLetter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
static bool isAlpha(char c) { return isLetter(c) || isDigit(c); }

/**
 * @brief read file content from `src` and write into string `input`
 *
 * @param src source file name
 * @param input content read from `src`
 * @return true if read and write successfully
 * @return false if file can not open
 */
static bool read_from(std::string src, std::string &input) {
  std::ifstream file(src);
  if (file.is_open() == false) {
    return false;
  }
  input = std::string((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return true;
}

/**
 * @brief write content `output` into file `dest`
 *
 * @param dest destination file name
 * @param input content will be writtened
 * @return true if write into file successfully
 * @return false if file can not open
 */
static bool write_to(std::string dest, std::string &output) {
  std::ofstream file(dest);
  if (file.is_open() == false) {
    return false;
  }
  file << output;
  file.close();
  return true;
}

static void trim(std::string &str) {
  /// erase leading whitespaces
  size_t start = str.find_first_not_of(" \t\n\r\f\v");
  if (start != std::string::npos) {
    str.erase(0, start);
  } else {
    /// string is all whitespace
    str.clear();
    return;
  }
  /// erase trailing whitespaces
  size_t end = str.find_last_not_of(" \t\n\r\f\v");
  if (end != std::string::npos) {
    str.erase(end + 1);
  }
}

static bool createIfNotExist(std::string filePath) {
  std::ofstream file(filePath, std::ios::out | std::ios::app);
  if (!file.is_open()) {
    return false;
  }
  file.close();
  return true;
}

} // namespace toyc

#endif
