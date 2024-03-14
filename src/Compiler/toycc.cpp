//! compiler for toyc

#include <Compiler/Compiler.h>
#include <Config.h>

int main(int argc, const char **argv) {
  if (argc < 2) {
    std::cerr << fstr("Usage: {} <src> <bytcode>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  toyc::Compiler compiler;
  /// wrap parameters
  std::string src = argv[1];
  if (!src.ends_with(toyc::ext)) {
    std::cerr << fstr("incorrect file extension\n");
    exit(EXIT_FAILURE);
  }
  std::string dest = (argc == 3) ? argv[2] : "a.ll";

  /// redirect to string first
  std::string output;
  llvm::raw_string_ostream ros(output);
  compiler.compile(src, ros);
  /// write into file
  if (toyc::write_to(dest, output) == false) {
    std::cerr << fstr("failed to open file '{}'\n", src);
    exit(EXIT_FAILURE);
  }
  return 0;
}
