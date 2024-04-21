#include "Util.h"
#include <Repl/LineEditor.h>

#include <cstdlib>
#include <optional>
#include <readline/history.h>
#include <readline/readline.h>

namespace toyc {

auto LineEditor::GetDefaultHistoryPath(const std::string &ProgName)
    -> std::string {
  std::string path = std::getenv("HOME");
  if (!path.empty()) {
    path.append("/." + ProgName + "-history");
    return path;
  }
  return path;
}

LineEditor::LineEditor(const std::string &ProgName,
                       const std::string &HistoryPath)
    : prompt_(ProgName + "> "), history_path_(HistoryPath) {
  if (HistoryPath.empty()) {
    history_path_ = GetDefaultHistoryPath(ProgName);
  }
  if (!CreateIfNotExist(history_path_)) {
    std::cerr << "failed to create file " << history_path_ << "\n";
    exit(EXIT_FAILURE);
  }
  LoadHistory();
}

LineEditor::~LineEditor() { SaveHistory(); }

auto LineEditor::ReadLine() -> std::optional<std::string> {
  char *input = readline(prompt_.c_str());
  if (input == nullptr) {
    return std::nullopt;
  }
  return std::string(input);
}

void LineEditor::SaveHistory() {
  if (!history_path_.empty()) {
    write_history(history_path_.c_str());
  }
}

void LineEditor::LoadHistory() {
  if (!history_path_.empty()) {
    read_history(history_path_.c_str());
  }
}

} // namespace toyc
