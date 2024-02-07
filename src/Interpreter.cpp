//! Interpreter implementation

#include <CodeGen.h>
#include <Interpreter.h>
#include <Parser.h>

#include <exception>

namespace toyc {

void Interpreter::compile(std::string &input) {
  /// support incremental parser
  parser.addInput(input);
  try {
    auto decl = parser.parse();
    if (decl != nullptr) {
      decl->dump();
    }
    auto ret = decl->codegen();
    if (ret != nullptr) {
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

  TheModule->print(llvm::errs(), nullptr);
}

} // namespace toyc
