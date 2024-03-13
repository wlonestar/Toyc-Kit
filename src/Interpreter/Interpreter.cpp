//! Interpreter implementation

#include <Interpreter/Interpreter.h>

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <memory>
#include <sstream>

namespace toyc {

void Interpreter::compile(std::string &input) {
  parser.addInput(input);
  try {
    auto unit = parser.parse();
    // std::cout << fstr("index={}\n", unit.index());
    if (unit.index() == 0) {
      debug("declaration");
      auto &decl = std::get<std::unique_ptr<Decl>>(unit);
      visitor.handleDeclaration(decl);
    } else if (unit.index() == 1) {
      debug("statement");
      auto &decl = std::get<std::unique_ptr<Stmt>>(unit);
    } else if (unit.index() == 2) {
      debug("expression");
      auto &expr = std::get<std::unique_ptr<Expr>>(unit);
      visitor.handleExpression(expr);
    } else {
      throw CodeGenException("error");
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
}

} // namespace toyc
