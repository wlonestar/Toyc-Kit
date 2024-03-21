//! compiler class implementation

#include <Compiler/Compiler.h>
#include <Preprocessor/Preprocessor.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>

namespace toyc {

void Compiler::compile(std::string &src, llvm::raw_ostream &os) {
  /// read from src file
  std::string input;
  if (read_from(src, input) == false) {
    std::cerr << fstr("failed to open file '{}'\n", src);
    exit(EXIT_FAILURE);
  }

  /// preprocessor
  preprocessor.setInput(input);
  try {
    input = preprocessor.process();
  } catch (PreprocessorException e) {
    std::cerr << e.what() << "\n";
    exit(EXIT_FAILURE);
  }

  /// parse
  parser.addInput(input);
  try {
    auto translationUnit = parser.parse();
    if (translationUnit != nullptr) {
#ifndef NDEBUG
      std::stringstream ss;
      translationUnit->dump(ss);
      std::cerr << ss.str();
#endif
      /// generate IR code
      visitor.setModuleID(src);
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
#ifdef NDEBUG
  } catch (...) {
    std::cerr << "there is something wrong in compiler inner\n";
    exit(EXIT_FAILURE);
#endif
  }

#ifndef NDEBUG
  visitor.dump();
#endif

  if (visitor.verifyModule() == false) {
    std::cerr << "there is something wrong in compiler inner\n";
    exit(EXIT_FAILURE);
  }
  visitor.dump(os);
}

} // namespace toyc
