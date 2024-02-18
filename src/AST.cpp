//! AST implementation

#include <AST.h>

namespace toyc {

/**
 * Expr
 */

std::string IntegerLiteral::getType() const { return type; }

std::string FloatingLiteral::getType() const { return type; }

std::string CharacterLiteral::getType() const { return "i64"; }

std::string StringLiteral::getType() const { return type; }

std::string DeclRefExpr::getType() const { return type; }

std::string ParenExpr::getType() const { return expr->getType(); }

std::string UnaryOperator::getType() const { return type; }

std::string BinaryOperator::getType() const { return type; }

/**
 * Decl
 */

std::string VarDecl::getType() const { return type; }

std::string ParamVarDecl::getType() const { return type; }

std::string FunctionDecl::getType() const { return type; }

} // namespace toyc
