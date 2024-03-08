//! compiler for toyc

#include <Compiler/Compiler.h>

using namespace toyc;
using namespace std;

void run_file(const char *arg0, std::string src, std::string dest) {
  Compiler compiler((char *)arg0);
  compiler.compile(src, dest);
}

int main(int argc, const char **argv) {
  if (argc < 2 || argc > 3) {
    cerr << "Usage: toyc <src> <bytcode>\n";
    exit(-1);
  }
  if (argc == 3) {
    run_file(argv[0], argv[1], argv[2]);
  } else if (argc == 2) {
    run_file(argv[0], argv[1], "a.ll");
  }
  return 0;
}
