//! Interpreter implementation

#include <Interpreter.h>
#include <Parser.h>

namespace toyc {

void Interpreter::compile(std::string &input) {
  // Lexer lexer(input);
  // for (;;) {
  //   try {
  //     Token token = lexer.scanToken();
  //     if (token.type == _EOF) {
  //       break;
  //     }
  //     debug("{}", token.toString());
  //   } catch (LexerError e) {
  //     std::cerr << e.what() << "\n";
  //     break;
  //   }
  // }

  Parser parser(input);
  try {
    auto expr = parser.parse();
    expr->dump();
  } catch (LexerError e) {
    std::cerr << e.what() << "\n";
    return;
  } catch (ParserError e) {
    std::cerr << e.what() << "\n";
    return;
  }
}

} // namespace toyc
