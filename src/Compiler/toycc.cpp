//! compiler for toyc

#include <Compiler/Compiler.h>

void execute(std::string src, std::string dest) {
  toyc::Compiler compiler;
  compiler.compile(src, dest);
}

int main(int argc, const char **argv) {
  if (argc < 2 || argc > 3) {
    std::cerr << "Usage: toyc <src> <bytcode>\n";
    exit(-1);
  }

  if (argc == 3) {
    execute(argv[1], argv[2]);
  } else if (argc == 2) {
    execute(argv[1], "a.ll");
  }
  return 0;
}
