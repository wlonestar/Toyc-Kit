//! Interpreter implementation

#include <Interpreter/Interpreter.h>

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <memory>
#include <sstream>

namespace toyc {

void Interpreter::Execute(InterpreterParser::ParseResult &unit) {
  if (unit.index() == 0) {
    auto &decl = std::get<std::unique_ptr<Decl>>(unit);
    visitor_.HandleDeclaration(decl);
  } else if (unit.index() == 1) {
    auto &stmt = std::get<std::unique_ptr<Stmt>>(unit);
    visitor_.HandleStatement(stmt);
  } else if (unit.index() == 2) {
    auto &expr = std::get<std::unique_ptr<Expr>>(unit);
    visitor_.HandleExpression(expr);
  } else {
    throw CodeGenException("error");
  }
}

void Interpreter::ParseAndExecute(std::string input) {
  parser_.AddInput(input);

  parser_.Advance();
  while (parser_.Peek().type_ != _EOF) {
    try {
      auto unit = parser_.Parse();
      Execute(unit); /// When catching error, report error resons and continue
    } catch (LexerException e1) {
      std::cerr << e1.what() << "\n";
      parser_.Advance();
    } catch (ParserException e2) {
      std::cerr << e2.what() << "\n";
      parser_.Advance();
    } catch (CodeGenException e3) {
      std::cerr << e3.what() << "\n";
      parser_.Advance();
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
