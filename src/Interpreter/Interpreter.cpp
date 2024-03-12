//! Interpreter implementation

#include "AST/AST.h"
#include <Interpreter/Interpreter.h>

#include <cstdlib>
#include <exception>
#include <memory>
#include <sstream>

namespace toyc {

void Interpreter::compile(std::string &input) {
  /// support incremental parser
  parser.addInput(input);
  try {
    using decl_t = std::unique_ptr<Decl>;
    using stmt_t = std::unique_ptr<Stmt>;
    using expr_t = std::unique_ptr<Expr>;
    auto unit = parser.parse();

    if (std::holds_alternative<decl_t>(unit) && !std::get<decl_t>(unit)) {
      auto &decl = std::get<std::unique_ptr<Decl>>(unit);
      decl->dump();
    } else if (std::holds_alternative<stmt_t>(unit) && !std::get<stmt_t>(unit)) {
      auto &decl = std::get<std::unique_ptr<Stmt>>(unit);
      decl->dump();
    } else if (std::holds_alternative<expr_t>(unit) && !std::get<expr_t>(unit)) {
      auto &decl = std::get<std::unique_ptr<Expr>>(unit);
      decl->dump();
    }
    // if (unit != nullptr) {
    //   visitor.codegen(*unit);
    // }
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

  // if (visitor.verifyModule() == false) {
  //   std::cerr << "there is something wrong in compiler inner\n";
  //   exit(EXIT_FAILURE);
  // }
}

} // namespace toyc
