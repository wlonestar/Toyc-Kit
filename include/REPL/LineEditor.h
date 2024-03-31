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
  std::string prompt_;
  size_t size_{};
  std::string file_path_;

public:
  LineEditor(std::string str, std::string _filePath)
      : prompt_(std::move(str)), file_path_(std::move(_filePath)) {
    if (!CreateIfNotExist(file_path_)) {
      std::cerr << makeString("failed to create file {}\n", file_path_);
      exit(EXIT_FAILURE);
    }
    if (read_history(file_path_.c_str()) != 0) {
      std::cerr << makeString("failed to read from {}\n", file_path_);
      exit(EXIT_FAILURE);
    }
  }

  ~LineEditor() {
    prompt_.clear();
    ClearHistory();
  }

public:
  void SetPrompt(std::string _prompt) { prompt_ = std::move(_prompt); }

  auto ReadLine() -> std::string { return {readline(prompt_.c_str())}; }

  void AddHistory(const std::string &str) {
    add_history(str.c_str());
    size_++;
  }

  void WriteHistory() {
    if (write_history(file_path_.c_str()) != 0) {
      std::cerr << makeString("failed to write into {}\n", file_path_);
      exit(EXIT_FAILURE);
    }
  }

  void ClearHistory() { remove_history(size_); }
};

} // namespace toyc

#endif
