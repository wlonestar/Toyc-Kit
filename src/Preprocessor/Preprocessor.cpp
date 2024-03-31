//! Toyc Preprocessor implementation

#include <Preprocessor/Preprocessor.h>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace toyc {

namespace fs = std::filesystem;

auto Preprocessor::IsEnd() -> bool { return current_ >= input_.size(); }

auto Preprocessor::Peek() -> char {
  if (IsEnd()) {
    return '\0';
  }
  return input_.at(current_);
}

auto Preprocessor::PeekNext() -> char {
  if (current_ + 1 >= input_.size()) {
    return '\0';
  }
  return input_.at(current_ + 1);
}

auto Preprocessor::Advance() -> char { return input_.at(current_++); }

void Preprocessor::Backward() { current_--; }

void Preprocessor::RemoveLineComment() {
  while (Peek() != '\n' && !IsEnd()) {
    Advance();
  }
  Backward();
  /// erase comment
  input_.erase(start_, current_ - start_ + 1);
  current_ = start_;
}

void Preprocessor::RemoveMutliComment() {
  Advance();
  /// line number the comment cross
  size_t line_cnt = 0;
  while (!(Peek() == '*' && PeekNext() == '/') && !IsEnd()) {
    if (Peek() == '\n') {
      line_++;
      col_ = 1;
      line_cnt++;
    }
    Advance();
  }
  if (IsEnd()) {
    ThrowPreprocessorException("unterminated /* comment");
  }
  Advance();
  if (Peek() != '/') {
    ThrowPreprocessorException("unterminated /* comment");
  }
  Advance();
  Backward();
  input_.erase(start_, current_ - start_ + 1);
  /// removed new line characters
  std::string new_line(line_cnt, '\n');
  /// insert new line characters again
  input_.insert(start_, new_line);
  start_ += line_cnt;
  current_ = start_;
}

void Preprocessor::ImportLib() {
  char c = Peek();
  while (Peek() != '\n' && !IsEnd()) {
    Advance();
  }
  /// peek out line start with `#`
  std::string line = input_.substr(start_, current_ - start_);
  if (line.starts_with("#include ")) {
    /// remove `include` macro
    input_.erase(start_, current_ - start_);
    /// find header file by name
    std::string lib_name = line.substr(line.find(' ') + 1);
    std::string path = "(not specified path)";
    /// get toycc path from environment variable
    if (auto e = getenv("toycc")) {
      path = std::string(e);
    }
    /// get absolute path of toycc
    auto absolute_path = fs::canonical(std::filesystem::path(path));
    /// find standard library files
    lib_name = absolute_path.parent_path().parent_path().string() +
               "/include/" + lib_name + ".toyc";

    /// read content from file
    std::string content;
    if (!ReadFrom(lib_name, content)) {
      std::cerr << makeString("failed to open file '{}'\n", lib_name);
      exit(EXIT_FAILURE);
    }
    /// recursivly process include file
    Preprocessor p;
    p.SetInput(content);
    content = p.Process();
    /// insert content into current file and reset cursors
    input_.insert(start_, content);
    start_ += content.size() + 1;
    current_ = start_;
  }
}

void Preprocessor::SetInput(std::string _input) { input_ = std::move(_input); }

auto Preprocessor::Process() -> std::string {
  while (Peek() != '\0') {
    char c = Peek();
    switch (c) {
    case '/':
      if (PeekNext() == '/') {
        RemoveLineComment();
      } else if (PeekNext() == '*') {
        RemoveMutliComment();
      } else {
        Advance();
        start_ = current_;
        continue;
      }
    case '#':
      ImportLib();
      continue;
    case '\n':
      line_++;
      Advance();
      start_ = current_;
      col_ = 0;
      break;
    default:
      Advance();
      start_ = current_;
      continue;
    }
  }
  return input_;
}

} // namespace toyc
