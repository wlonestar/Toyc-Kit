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

void Compiler::Compile(std::string &src, llvm::raw_ostream &os) {
  /// read from src file
  std::string input;
  if (!ReadFrom(src, input)) {
    std::cerr << makeString("failed to open file '{}'\n", src);
    exit(EXIT_FAILURE);
  }

  /// preprocessor
  preprocessor_.SetInput(input);
  try {
    input = preprocessor_.Process();
  } catch (PreprocessorException e) {
    std::cerr << e.what() << "\n";
    exit(EXIT_FAILURE);
  }

  /// parse
  parser_.AddInput(input);
  try {
    auto translation_unit = parser_.Parse();
    if (translation_unit != nullptr) {
#ifndef NDEBUG
      std::stringstream ss;
      translation_unit->Dump(ss);
      std::cerr << ss.str();
#endif
      /// generate IR code
      visitor_.SetModuleID(src);
      visitor_.Codegen(*translation_unit);
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
  visitor_.Dump();
#endif

  if (!visitor_.VerifyModule()) {
    std::cerr << "there is something wrong in compiler inner\n";
    exit(EXIT_FAILURE);
  }
  visitor_.Dump(os);
}

} // namespace toyc
