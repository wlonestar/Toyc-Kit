//! entry point of toyc

#include <Config.h>
#include <Interpreter/Interpreter.h>
#include <Repl/LineEditor.h>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

auto main(int argc, const char **argv) -> int {
  toyc::Interpreter interpreter;

  toyc::LineEditor editor("toyc-repl");
  std::string input;
  while (auto line = editor.ReadLine()) {
    /// line buffer
    std::string l = *line;
    /// only compile non-empty string
    toyc::Trim(l);
    if (l.ends_with("\\")) {
      /// multiple input
      l.pop_back();
      input += l;
      editor.SetPrompt(".......... ");
      continue;
    }

    input += l;

    /// deal with options (now only support quit)
    if (input == R"(.quit)") {
      std::cout << "bye~\n";
      break;
    }

    interpreter.ParseAndExecute(input);

    /// clear buffer
    input = "";
    editor.SetPrompt("toyc-repl> ");
  }
  return 0;
}
