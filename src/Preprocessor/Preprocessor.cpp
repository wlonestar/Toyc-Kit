//! preprocessor implementation

#include "Util.h"
#include <Preprocessor/Preprocessor.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace toyc {

namespace fs = std::filesystem;

bool Preprocessor::isEnd() { return current >= input.size(); }

char Preprocessor::peek() {
  if (isEnd()) {
    return '\0';
  }
  return input.at(current);
}

char Preprocessor::peekNext() {
  if (current + 1 >= input.size()) {
    return '\0';
  }
  return input.at(current + 1);
}

char Preprocessor::advance() { return input.at(current++); }

void Preprocessor::backward() { current--; }

void Preprocessor::skipLineComment() {
  while (peek() != '\n' && !isEnd()) {
    advance();
  }
  backward();
  input.erase(start, current - start + 1);
  current = start;
}

void Preprocessor::skipMutliComment() {
  advance();
  size_t l = 0;
  while (!(peek() == '*' && peekNext() == '/') && !isEnd()) {
    if (peek() == '\n') {
      line++;
      col = 1;
      l++;
    }
    advance();
  }
  if (isEnd()) {
    throwPreprocessorException("unterminated /* comment");
  }
  advance();
  if (peek() != '/') {
    throwPreprocessorException("unterminated /* comment");
  }
  advance();
  backward();
  input.erase(start, current - start + 1);
  /// add remove new line characters
  std::string newl(l, '\n');
  input.insert(start, newl);
  start += l;
  current = start;
}

void Preprocessor::importLib() {
  char c = peek();
  while (peek() != '\n' && !isEnd()) {
    advance();
  }
  std::string _line = input.substr(start, current - start);

  if (_line.starts_with("#include ")) {
    /// remove `include` macro
    input.erase(start, current - start);
    /// find header file by name
    std::string libName = _line.substr(_line.find(" ") + 1);
    std::string path = "toycc";
    if (auto e = getenv("toycc")) {
      path = std::string(e);
    }
    auto absolutePath = fs::canonical(std::filesystem::path(path));
    libName = absolutePath.parent_path().parent_path().string() + "/include/" +
              libName + ".toyc";

    /// read content from file
    std::string content;
    if (read_from(libName, content) == false) {
      std::cerr << fstr("failed to open file '{}'\n", libName);
      exit(EXIT_FAILURE);
    }
    /// recursivly process include file
    Preprocessor p;
    p.setInput(content);
    content = p.process();

    /// insert content into current file and reset cursors
    input.insert(start, content);
    start += content.size() + 1;
    current = start;
    // } else {
    // return;
  }
}

std::string Preprocessor::process() {
  while (peek() != '\0') {
    char c = peek();
    switch (c) {
    case '/':
      if (peekNext() == '/') {
        skipLineComment();
      } else if (peekNext() == '*') {
        skipMutliComment();
      } else {
        advance();
        start = current;
        continue;
      }
    case '#':
      importLib();
      continue;
    case '\n':
      line++;
      advance();
      start = current;
      col = 0;
      break;
    default:
      advance();
      start = current;
      continue;
    }
  }
  return input;
}

} // namespace toyc
