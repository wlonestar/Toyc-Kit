//! AST Print

#include <AST.h>
#include <ASTPrint.h>
#include <Util.h>

#include <iostream>

namespace toyc {

void IntegerLiteral::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_SYNTAX("IntegerLiteral") << " "
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
            << AST_LEADER("-") << AST_SYNTAX("FloatingLiteral") << " "
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
            << AST_LEADER("-") << AST_SYNTAX("CharacterLiteral") << " "
            << AST_TYPE("'{}'", getType()) << " " << AST_LITERAL("{}", value)
            << "\n";
}

void StringLiteral::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_SYNTAX("StringLiteral") << " "
            << AST_TYPE("'{}'", getType()) << " " << AST_LITERAL("\"{}\"", value)
            << "\n";
}

void ParenExpr::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_SYNTAX("ParenExpr") << "\n";
  expr->dump(_d + 1, LEAF, _p + "  ");
}

void UnaryOperator::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_SYNTAX("UnaryOperator") << " "
            << fstr("'{}'", op.value) << "\n";
  right->dump(_d + 1, LEAF, _p + "  ");
}

void BinaryOperator::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_SYNTAX("BinaryOperator") << " "
            << fstr("'{}'", op.value) << "\n";
  left->dump(_d + 1, INTERNAL, _p + "  |");
  right->dump(_d + 1, LEAF, _p + "  ");
}

} // namespace toyc
