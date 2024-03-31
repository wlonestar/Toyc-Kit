//! lightweight wrapped readline with a more friendly API.

#ifndef LINE_EDITOR_H
#define LINE_EDITOR_H

#pragma once

#include <readline/history.h>
#include <readline/readline.h>

#include <string>

#include <Util.h>

namespace toyc {

class LineEditor {
private:
  std::string prompt;
  size_t size;
  std::string filePath;

public:
  LineEditor(std::string str, std::string _filePath)
      : prompt(str), size(0), filePath(_filePath) {
    std::cout << filePath << "\n";
    if (!createIfNotExist(filePath)) {
      std::cerr << makeString("failed to create file {}\n", filePath);
      exit(EXIT_FAILURE);
    }
    if (read_history(filePath.c_str()) != 0) {
      std::cerr << makeString("failed to read from {}\n", filePath);
      exit(EXIT_FAILURE);
    }
  }

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

  void writeHistory() {
    if (write_history(filePath.c_str()) != 0) {
      std::cerr << makeString("failed to write into {}\n", filePath);
      exit(EXIT_FAILURE);
    }
  }

  void clearHistory() { remove_history(size); }
};

} // namespace toyc

#endif
