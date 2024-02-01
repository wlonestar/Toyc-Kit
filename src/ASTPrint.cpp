//! AST Print

#include <AST.h>
#include <ASTPrint.h>

#include <iostream>

namespace toyc {

void IntegerLiteral::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_SYNTAX("IntegerLiteral") << " "
            << AST_TYPE("'int'") << " " << AST_LITERAL(std::to_string(value))
            << "\n";
}

void FloatingLiteral::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_SYNTAX("FloatingLiteral") << " "
            << AST_TYPE("'double'") << " " << AST_LITERAL(std::to_string(value))
            << "\n";
}

void StringLiteral::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_SYNTAX("StringLiteral") << " "
            << AST_TYPE("'char *'") << " "
            << AST_LITERAL(fmt_str("\"{}\"", value)) << "\n";
}

void ParenExpr::dump(size_t _d, Side _s, std::string _p) {
  std::cout << (_d == 0 ? AST_LEADER("`")
                        : (_s == LEAF ? _p + AST_LEADER("`") : _p))
            << AST_LEADER("-") << AST_SYNTAX("ParenExpr") << "\n";
  // << AST_TYPE("'" + DataTypeTable[type] + "'") << "\n";
  expr->dump(_d + 1, LEAF, _p + "  ");
}

} // namespace toyc
