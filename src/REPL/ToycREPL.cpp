//! entry point of toyc

#include <Config.h>
#include <Interpreter/Interpreter.h>
#include <REPL/LineEditor.h>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

auto main(int argc, const char **argv) -> int {
  std::string input;
  toyc::Interpreter interpreter;
  std::string file_path = std::string(std::getenv("HOME")) + "/" + HISTORY_FILE;
  toyc::LineEditor editor(toyc::prompt, file_path);
  std::string line;
  while (true) {
    /// line buffer
    line = editor.ReadLine();
    toyc::Trim(line);

    /// only compile non-empty string
    if (!line.empty()) {
      editor.AddHistory(line);
      /// deal with options (now only support quit)
      if (line == R"(.exit)") {
        /// exit
        std::cout << "bye~\n";
        break;
      }
      /// normal statements
      if (line.ends_with("\\")) {
        /// multiple input
        line.pop_back();
        input += line;
        editor.SetPrompt(toyc::multi_prompt);
        continue;
      }
      /// single input
      editor.SetPrompt(toyc::prompt);
      input += line;
      interpreter.ParseAndExecute(input);
      editor.WriteHistory();

      /// clear buffer
      input = "";
    }
  }
  return 0;
}
