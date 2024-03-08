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

bool Compiler::readFrom(std::string &src, std::string &input) {
  std::ifstream file(src);
  if (file.is_open() == false) {
    return false;
  }
  input = std::string((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return true;
}

bool Compiler::writeTo(std::string &dest) {
  int fd;
  std::error_code ec = llvm::sys::fs::openFileForWrite(
      dest, fd, llvm::sys::fs::CD_CreateAlways, llvm::sys::fs::OF_None);
  if (ec) {
    return false;
  }
  llvm::raw_fd_ostream file(fd, true);
  visitor.dump(file);
  return true;
}

void Compiler::compile(std::string &src, std::string &dest) {
  /// read from src file
  std::string input;
  if (readFrom(src, input) == false) {
    std::cerr << fstr("failed to open file '{}'\n", src);
    exit(EXIT_FAILURE);
  }

  /// preprocessor
  preprocessor.addInput(input);
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
#ifdef DEBUG
      std::stringstream ss;
      translationUnit->dump(ss);
      std::cout << ss.str();
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
#ifndef DEBUG
  } catch (...) {
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
  if (writeTo(dest) == false) {
    std::cerr << fstr("failed to open file '{}'\n", dest);
    exit(EXIT_FAILURE);
  }
}

} // namespace toyc
