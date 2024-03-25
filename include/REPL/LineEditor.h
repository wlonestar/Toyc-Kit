//! lightweight wrapped readline with a more friendly API.

#ifndef LINE_EDITOR_H
#define LINE_EDITOR_H

#pragma once

#include <readline/history.h>
#include <readline/readline.h>

#include <string>

namespace toyc {

class LineEditor {
private:
  std::string prompt;
  size_t size;

public:
  LineEditor(std::string str) : prompt(str), size(0) {}

  ~LineEditor() {
    prompt.clear();
    clearHistory();
  }

public:
  void setPrompt(std::string _prompt) { prompt = _prompt; }

  std::string readLine() { return std::string(readline(prompt.c_str())); }

  void addHistory(std::string str) {
    add_history(str.c_str());
    size++;
  }

  void clearHistory() { remove_history(size); }
};

} // namespace toyc

#endif
