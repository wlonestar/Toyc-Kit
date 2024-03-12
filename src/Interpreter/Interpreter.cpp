//! Interpreter implementation

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
    // using decl_t = std::unique_ptr<Decl>;
    // using stmt_t = std::unique_ptr<Stmt>;
    // using expr_t = std::unique_ptr<Expr>;
    auto unit = parser.parse();
    std::cout << fstr("index={}\n", unit.index());

    if (unit.index() == 0) {
      debug("declaration");
      auto &decl = std::get<std::unique_ptr<Decl>>(unit);
      visitor.handleDeclaration(decl);
    } else if (unit.index() == 1) {
      debug("statement");
      auto &decl = std::get<std::unique_ptr<Stmt>>(unit);
    } else if (unit.index() == 2) {
      debug("expression");
      std::string type = std::get<std::unique_ptr<Expr>>(unit)->getType();
      auto stmt = std::make_unique<ReturnStmt>(
          std::move(std::get<std::unique_ptr<Expr>>(unit)));
      auto proto = std::make_unique<FunctionProto>(
          "__anon_expr", std::move(type),
          std::vector<std::unique_ptr<ParmVarDecl>>{});
      auto fn =
          std::make_unique<FunctionDecl>(std::move(proto), std::move(stmt));

      visitor.handleExpression(fn);
    } else {
      throw CodeGenException("error");
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
