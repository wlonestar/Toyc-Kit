//! lightweight wrapped readline with a more friendly API.

#ifndef LINE_EDITOR_H
#define LINE_EDITOR_H

#pragma once

#include <optional>
#include <string>
#include <utility>

#include <Util.h>

namespace toyc {

class LineEditor {
private:
  std::string prompt_;
  std::string history_path_;

  static auto GetDefaultHistoryPath(const std::string &ProgName) -> std::string;

public:
  explicit LineEditor(const std::string &ProgName, const std::string &HistoryPath = "");
  ~LineEditor();

  auto ReadLine() -> std::optional<std::string>;

  auto GetPrompt() const -> const std::string & { return prompt_; }
  void SetPrompt(std::string prompt) { prompt_ = std::move(prompt); }

  void SaveHistory();
  void LoadHistory();
};

} // namespace toyc

#endif
