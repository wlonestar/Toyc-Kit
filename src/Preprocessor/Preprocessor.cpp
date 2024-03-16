//! Toyc Preprocessor implementation

#include <Preprocessor/Preprocessor.h>

#include <filesystem>
#include <fstream>
#include <iostream>

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

void Preprocessor::removeLineComment() {
  while (peek() != '\n' && !isEnd()) {
    advance();
  }
  backward();
  /// erase comment
  input.erase(start, current - start + 1);
  current = start;
}

void Preprocessor::removeMutliComment() {
  advance();
  /// line number the comment cross
  size_t lineCnt = 0;
  while (!(peek() == '*' && peekNext() == '/') && !isEnd()) {
    if (peek() == '\n') {
      line++;
      col = 1;
      lineCnt++;
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
  /// removed new line characters
  std::string newLine(lineCnt, '\n');
  /// insert new line characters again
  input.insert(start, newLine);
  start += lineCnt;
  current = start;
}

void Preprocessor::importLib() {
  char c = peek();
  while (peek() != '\n' && !isEnd()) {
    advance();
  }
  /// peek out line start with `#`
  std::string _line = input.substr(start, current - start);
  if (_line.starts_with("#include ")) {
    /// remove `include` macro
    input.erase(start, current - start);
    /// find header file by name
    std::string libName = _line.substr(_line.find(" ") + 1);
    std::string path = "(not specified path)";
    /// get toycc path from environment variable
    if (auto e = getenv("toycc")) {
      path = std::string(e);
    }
    /// get absolute path of toycc
    auto absolutePath = fs::canonical(std::filesystem::path(path));
    /// find standard library files
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
  }
}

void Preprocessor::setInput(std::string _input) { input = _input; }

std::string Preprocessor::process() {
  while (peek() != '\0') {
    char c = peek();
    switch (c) {
    case '/':
      if (peekNext() == '/') {
        removeLineComment();
      } else if (peekNext() == '*') {
        removeMutliComment();
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
