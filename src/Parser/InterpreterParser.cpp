//! Parser implementation

#include <AST/AST.h>
#include <CodeGen/InterpreterCodeGen.h>
#include <Lexer/Token.h>
#include <Parser/InterpreterParser.h>
#include <Util.h>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace toyc {

std::unique_ptr<Decl>
InterpreterParser::parseVariableDeclaration(std::string &type,
                                            std::string &name, VarScope scope) {
  if (varTable.find(name) != varTable.end()) {
    throwParserException(fstr("redefinition of '{}'", name));
  }
  varTable[name] = type;

  std::unique_ptr<Expr> init =
      (match(EQUAL) ? parseAssignmentExpression() : nullptr);
  if (init != nullptr && type != init->getType()) {
    init = std::make_unique<ImplicitCastExpr>(type, std::move(init));
  }
  // std::unique_ptr<VarDecl> decl =
  //     std::make_unique<VarDecl>(name, type, std::move(init), scope);
  consume(SEMI, "expected ';' after declaration");

  auto stmt = std::make_unique<ReturnStmt>(std::move(init));

  std::string funcName = "__anon_expr";
  std::vector<std::unique_ptr<ParmVarDecl>> params;
  /// wrap into a function
  auto func =
      std::make_unique<FunctionDecl>(funcName, type, params, std::move(stmt));
  funcTable[funcName] = std::make_pair(type, std::vector<std::string>{});
  clearVarTable();
  return func;
}

std::unique_ptr<Decl> InterpreterParser::parseExternalDeclaration() {
  auto [type, flag] = parseDeclarationSpecifiers();
  auto name = parseDeclarator();
  if (match(LP)) {
    return parseFunctionDeclaration(type, name, flag);
  } else {
    return parseVariableDeclaration(type, name, LOCAL);
  }
}

std::unique_ptr<TranslationUnitDecl> InterpreterParser::parse() {
  std::vector<std::unique_ptr<Decl>> decls;
  advance();
  while (current.type != _EOF) {
    auto decl = parseExternalDeclaration();
    decls.push_back(std::move(decl));
  }
  return std::make_unique<TranslationUnitDecl>(std::move(decls));
}

} // namespace toyc
