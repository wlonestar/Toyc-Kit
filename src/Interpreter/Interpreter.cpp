//! Interpreter implementation

#include <Interpreter/Interpreter.h>

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <memory>
#include <sstream>

namespace toyc {

void Interpreter::execute(InterpreterParser::ParseResult &unit) {
  if (unit.index() == 0) {
    auto &decl = std::get<std::unique_ptr<Decl>>(unit);
    visitor.handleDeclaration(decl);
  } else if (unit.index() == 1) {
    auto &stmt = std::get<std::unique_ptr<Stmt>>(unit);
    visitor.handleStatement(stmt);
  } else if (unit.index() == 2) {
    auto &expr = std::get<std::unique_ptr<Expr>>(unit);
    visitor.handleExpression(expr);
  } else {
    throw CodeGenException("error");
  }
}

void Interpreter::parseAndExecute(std::string input) {
  parser.addInput(input);

  parser.advance();
  while (parser.peek().type != _EOF) {
    try {
      auto unit = parser.parse();
      execute(unit); /// When catching error, report error resons and continue
    } catch (LexerException e1) {
      std::cerr << e1.what() << "\n";
      parser.advance();
    } catch (ParserException e2) {
      std::cerr << e2.what() << "\n";
      parser.advance();
    } catch (CodeGenException e3) {
      std::cerr << e3.what() << "\n";
      parser.advance();
#ifndef DEBUG
    } catch (...) {
      /// If an unexpected error occurs, quit the program
      std::cerr << "there is something wrong in compiler inner\n";
      exit(EXIT_FAILURE);
#endif
    }
  }
}

} // namespace toyc
