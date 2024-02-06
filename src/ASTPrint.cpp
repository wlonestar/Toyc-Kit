//! AST Print

#include <AST.h>
#include <ASTPrint.h>
#include <Util.h>

#include <cstddef>
#include <iostream>

namespace toyc {

/**
 * Expr
 */

void IntegerLiteral::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_STMT("IntegerLiteral") << " "
            << AST_TYPE("'{}'", type) << " ";
  if (type == "int") {
    std::cout << AST_LITERAL("{}", std::get<0>(value)) << "\n";
  } else if (type == "unsigned int") {
    std::cout << AST_LITERAL("{}", std::get<1>(value)) << "\n";
  } else if (type == "long") {
    std::cout << AST_LITERAL("{}", std::get<2>(value)) << "\n";
  } else if (type == "unsigned long") {
    std::cout << AST_LITERAL("{}", std::get<3>(value)) << "\n";
  } else if (type == "long long") {
    std::cout << AST_LITERAL("{}", std::get<4>(value)) << "\n";
  } else if (type == "unsigned long long") {
    std::cout << AST_LITERAL("{}", std::get<5>(value)) << "\n";
  } else {
    /// TODO: support more types
    std::cout << AST_LITERAL("{}", std::get<0>(value)) << "\n";
  }
}

void FloatingLiteral::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_STMT("FloatingLiteral") << " "
            << AST_TYPE("'{}'", type) << " ";
  if (type == "float") {
    std::cout << AST_LITERAL("{}", std::get<0>(value)) << "\n";
  } else if (type == "double") {
    std::cout << AST_LITERAL("{}", std::get<1>(value)) << "\n";
  } else if (type == "long double") {
    std::cout << AST_LITERAL("{}", std::get<2>(value)) << "\n";
  }
}

void CharacterLiteral::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_STMT("CharacterLiteral") << " "
            << AST_TYPE("'{}'", getType()) << " " << AST_LITERAL("{}", value)
            << "\n";
}

void StringLiteral::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_STMT("StringLiteral") << " "
            << AST_TYPE("'{}'", getType()) << " "
            << AST_LITERAL("\"{}\"", value) << "\n";
}

void DeclRefExpr::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_STMT("DeclRefExpr") << " "
            << AST_TYPE("'{}'", getType()) << " " << AST_LITERAL("'{}'", name)
            << "\n";
}

void ParenExpr::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_STMT("ParenExpr") << "\n";
  expr->dump(_d + 1, LEAF, _p + "  ");
}

void UnaryOperator::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_STMT("UnaryOperator") << " "
            << fstr("'{}'", op.value) << "\n";
  right->dump(_d + 1, LEAF, _p + "  ");
}

void BinaryOperator::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_STMT("BinaryOperator") << " "
            << AST_TYPE("{}", getType()) << " " << fstr("'{}'", op.value)
            << "\n";
  left->dump(_d + 1, INTERNAL, _p + "  |");
  right->dump(_d + 1, LEAF, _p + "  ");
}

/**
 * Decl
 */

void VarDecl::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_DECL("VarDecl") << " "
            << AST_LITERAL("'{}'", name) << " " << AST_TYPE("{}", getType())
            << "\n";
  if (init != nullptr) {
    init->dump(_d + 1, LEAF, _p + "  ");
  }
}

} // namespace toyc
