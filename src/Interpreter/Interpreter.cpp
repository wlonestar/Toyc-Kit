//! Interpreter implementation

#include <CodeGen/InterpreterCodeGen.h>
#include <Interpreter/Interpreter.h>
#include <Parser/InterpreterParser.h>

#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <cstdlib>
#include <exception>
#include <sstream>

namespace toyc {

void Interpreter::compile(std::string &input) {
  /// support incremental parser
  parser.addInput(input);
  try {
    auto translationUnit = parser.parse();
    if (translationUnit != nullptr) {
#ifdef DEBUG
      std::stringstream ss;
      translationUnit->dump(ss);
      std::cout << ss.str();
#endif
      /// generate IR code
      visitor.codegen(*translationUnit);
    }
  } catch (LexerException e1) {
    std::cerr << e1.what() << "\n";
    exit(EXIT_FAILURE);
  } catch (ParserException e2) {
    std::cerr << e2.what() << "\n";
    exit(EXIT_FAILURE);
  } catch (CodeGenException e3) {
    std::cerr << e3.what() << "\n";
    exit(EXIT_FAILURE);
#ifndef DEBUG
  } catch (...) {
    /// catch error for release version
    std::cerr << "there is something wrong in compiler inner\n";
    exit(EXIT_FAILURE);
#endif
  }

#ifdef DEBUG
  visitor.dump();
#endif

  if (visitor.verifyModule() == false) {
    std::cerr << "there is something wrong in compiler inner\n";
    exit(EXIT_FAILURE);
  }
}

} // namespace toyc
