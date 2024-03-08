//! preprocessor implementation

#include <Preprocessor/Preprocessor.h>

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

void Preprocessor::skipMutliComment() {
  input[current] = ' ';
  advance();
  while (!(peek() == '*' && peekNext() == '/') && !isEnd()) {
    if (peek() == '\n') {
      line++;
      col = 1;
    } else {
      input[current] = ' ';
    }
    advance();
  }
  if (isEnd()) {
    throwPreprocessorException("unterminated /* comment");
  }
  input[current] = ' ';
  advance();
  if (peek() != '/') {
    throwPreprocessorException("unterminated /* comment");
  }
  input[current] = ' ';
  advance();
}

bool Preprocessor::readFrom(std::string &src, std::string &input) {
  std::ifstream file(src);
  if (file.is_open() == false) {
    return false;
  }
  input = std::string((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return true;
}

void Preprocessor::importLib() {
  char c = peek();
  while (peek() != '\n' && !isEnd()) {
    advance();
  }
  std::string _line = input.substr(start, current - start);

  if (_line.starts_with("#include ")) {
    input.erase(start, current - start);

    std::string libName = _line.substr(_line.find(" ") + 1);

    auto execPath = fs::canonical(std::filesystem::path(arg0));
    libName = execPath.parent_path().parent_path().string() + "/include/" +
              libName + ".toyc";

    std::string content;
    if (readFrom(libName, content) == false) {
      std::cerr << fstr("failed to open file '{}'\n", libName);
      exit(EXIT_FAILURE);
    }

    input.insert(start, content);
    start += content.size() + 1;
    current = start;
  } else {
    return;
  }
}

void Preprocessor::addInput(std::string &_input) { input = _input; }

std::string Preprocessor::process() {
  while (peek() != '\0') {
    char c = peek();
    switch (c) {
    case '/':
      if (peekNext() == '/') {
        input[current] = ' ';
        while (peek() != '\n' && !isEnd()) {
          input[current] = ' ';
          advance();
        }
      } else if (peekNext() == '*') {
        skipMutliComment();
      } else {
        start = current;
        advance();
        continue;
      }
    case '#':
      importLib();
      continue;
    case '\n':
      line++;
      start = current;
      advance();
      col = 0;
      break;
    default:
      start = current;
      advance();
      continue;
    }
  }
  return input;
}

} // namespace toyc
