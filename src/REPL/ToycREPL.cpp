//! entry point of toyc

#include <Config.h>
#include <Interpreter/Interpreter.h>
#include <REPL/LineEditor.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

using namespace toyc;
using namespace std;

int main(int argc, const char **argv) {
  string input;
  Interpreter interpreter;
  LineEditor editor(PROMPT);
  string line;
  while (true) {
    /// line buffer
    line = editor.readLine();
    trim(line);

    /// only compile non-empty string
    if (line.size() > 0) {
      editor.addHistory(line);
      /// deal with options (now only support quit)
      if (line == R"(.exit)") {
        /// exit
        cout << "bye~" << endl;
        break;
      }
      /// normal statements
      if (line.ends_with("\\")) {
        /// multiple input
        line.pop_back();
        input += line;
        editor.setPrompt(MULTI_PROMPT);
        continue;
      } else {
        /// single input
        editor.setPrompt(PROMPT);
        input += line;
        interpreter.parseAndExecute(input);

        /// clear buffer
        input = "";
      }
    }
  }
  return 0;
}
