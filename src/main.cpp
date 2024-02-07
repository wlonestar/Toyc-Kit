//! entry point of toyc

#include <Interpreter.h>
#include <Lexer.h>
#include <LineEditor.h>
#include <Token.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

using namespace toyc;
using namespace std;

void run_file(const char *filename) {
  ifstream file(filename);
  if (!file.is_open()) {
    cerr << "Failed to open file '" << filename << "'\n";
    exit(-1);
  }
  string input((std::istreambuf_iterator<char>(file)),
               std::istreambuf_iterator<char>());
  file.close();
  Interpreter interpreter;
  interpreter.compile(input);
}

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
    cerr << "Usage: toyc <script>\n";
    exit(-1);
  } else if (argc == 2) {
    run_file(argv[1]);
  } else {
    run_prompt();
  }
  return 0;
}
