//! entry point of toyc

#include <Lexer.h>
#include <Token.h>
#include <interpreter/Interpreter.h>
#include <interpreter/LineEditor.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

using namespace toyc;
using namespace std;

void run_prompt() {
  string input;
  Interpreter interpreter;
  LineEditor editor(PROMPT);
  while (true) {
    /// line buffer
    string line = editor.readLine();
    trim(line);

    /// only compile non-empty string
    if (line.size() > 0) {
      editor.addHistory(line);
      /// deal with options (now only support quit)
      if (line == R"(.quit)") {
        /// quit
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

        interpreter.compile(input);

        /// clear buffer
        input = "";
      }
    }
  }
}

int main(int argc, const char **argv) {
  if (argc > 2) {
    cerr << "Usage: toyc <src> <bytcode>\n";
    exit(-1);
  }
  run_prompt();
  return 0;
}
