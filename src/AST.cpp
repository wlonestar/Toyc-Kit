//! AST implementation

#include <AST.h>

namespace toyc {

/**
 * Expr
 */

std::string IntegerLiteral::getType() const { return type; }

std::string FloatingLiteral::getType() const { return type; }

std::string CharacterLiteral::getType() const { return "int"; }

std::string StringLiteral::getType() const { return type; }

std::string DeclRefExpr::getType() const { return type; }

std::string ParenExpr::getType() const { return ""; }

std::string UnaryOperator::getType() const { return ""; }

std::string BinaryOperator::getType() const { return type; }

/**
 * Decl
 */

std::string VarDecl::getType() const { return type; }

/**
 * TranslationUnitDecl
 */

std::string TranslationUnitDecl::getType() const { return ""; }

} // namespace toyc
