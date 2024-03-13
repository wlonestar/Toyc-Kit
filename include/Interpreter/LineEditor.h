//! lightweight wrapped readline with a more friendly API.

#ifndef LINE_EDITOR_H
#define LINE_EDITOR_H

#pragma once

#include <readline/history.h>
#include <readline/readline.h>

#include <string>

namespace toyc {

static std::string PROMPT = "toyci> ";
static std::string MULTI_PROMPT = "...... ";

class LineEditor {
private:
  std::string _prompt;
  size_t _size;

public:
  LineEditor(std::string &str) : _prompt(str), _size(0) {}
  ~LineEditor() { clearHistory(); }

  void setPrompt(std::string &prompt) { _prompt = prompt; }
  std::string readLine() { return std::string(readline(_prompt.c_str())); }
  void addHistory(std::string &str) {
    add_history(str.c_str());
    _size++;
  }
  void clearHistory() { remove_history(_size); }
};

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

} // namespace toyc

#endif
