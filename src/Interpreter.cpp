//! Interpreter implementation

#include <CodeGen.h>
#include <Interpreter.h>
#include <Parser.h>

#include <exception>

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

  initializeModule();

  Parser parser(input);
  try {
    auto decl = parser.parse();
    if (decl != nullptr) {
      debug("parse result");
      decl->dump();
    }
    auto ret = decl->codegen();
    if (ret != nullptr) {
      debug("codegen result");
      ret->dump();
    }
  } catch (LexerError e) {
    std::cerr << e.what() << "\n";
    return;
  } catch (ParserError e) {
    std::cerr << e.what() << "\n";
    return;
  } catch (...) {
    std::cerr << "there is something wrong in compiler inner\n";
    return;
  }

  // TheModule->print(llvm::errs(), nullptr);
}

} // namespace toyc
