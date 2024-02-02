//! AST implementation

#include <AST.h>

namespace toyc {

std::string IntegerLiteral::getType() const { return type; }

std::string FloatingLiteral::getType() const { return type; }

std::string StringLiteral::getType() const { return type; }

std::string ParenExpr::getType() const { return ""; }

std::string UnaryOperator::getType() const { return ""; }

std::string BinaryOperator::getType() const { return ""; }

} // namespace toyc
