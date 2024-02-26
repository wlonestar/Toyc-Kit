//! AST Print

#include <AST.h>
#include <ASTPrint.h>
#include <Util.h>

#include <cstddef>
#include <ostream>

namespace toyc {

void printASTLeader(std::ostream &os, size_t _d, Side _s, std::string _p) {
  os << AST_LEADER("{}-", (_d == 0 ? "`" : (_s == LEAF ? _p + "`" : _p)));
}

/**
 * Expr
 */

void IntegerLiteral::dump(std::ostream &os, size_t _d, Side _s,
                          std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}\n", AST_STMT("IntegerLiteral"), AST_TYPE("'{}'", type),
             AST_LITERAL("{}", value));
}

void FloatingLiteral::dump(std::ostream &os, size_t _d, Side _s,
                           std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}\n", AST_STMT("FloatingLiteral"), AST_TYPE("'{}'", type),
             AST_LITERAL("{}", value));
}

void CharacterLiteral::dump(std::ostream &os, size_t _d, Side _s,
                            std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}\n", AST_STMT("CharacterLiteral"), AST_TYPE("'{}'", type),
             AST_LITERAL("{}", value));
}

void StringLiteral::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}\n", AST_STMT("StringLiteral"), AST_TYPE("'{}'", type),
             AST_LITERAL("\"{}\"", value));
}

void DeclRefExpr::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}\n", AST_STMT("DeclRefExpr"), AST_TYPE("'{}'", type),
             AST_LITERAL("'{}'", name));
}

void ParenExpr::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {}\n", AST_STMT("ParenExpr"), AST_TYPE("'{}'", getType()));
  expr->dump(os, _d + 1, LEAF, _p + "  ");
}

void UnaryOperator::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} '{}'\n", AST_STMT("UnaryOperator"), op.value);
  right->dump(os, _d + 1, LEAF, _p + "  ");
}

void BinaryOperator::dump(std::ostream &os, size_t _d, Side _s,
                          std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} '{}'\n", AST_STMT("BinaryOperator"), AST_TYPE("'{}'", type),
             op.value);
  left->dump(os, _d + 1, INTERNAL, _p + "  |");
  right->dump(os, _d + 1, LEAF, _p + "  ");
}

/**
 * Stmt
 */

void CompoundStmt::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{}\n", AST_STMT("CompoundStmt"));
  if (stmts.size() == 1) {
    stmts[0]->dump(os, _d + 1, LEAF, _p + "  ");
  } else if (stmts.size() >= 2) {
    for (int i = 0; i < stmts.size() - 1; i++) {
      stmts[i]->dump(os, _d + 1, INTERNAL, _p + "  |");
    }
    stmts[stmts.size() - 1]->dump(os, _d + 1, LEAF, _p + "  ");
  }
}

void ExprStmt::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{}\n", AST_STMT("ExprStmt"));
  if (expr != nullptr) {
    expr->dump(os, _d + 1, LEAF, _p + "  ");
  }
}

void DeclStmt::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{}\n", AST_STMT("DeclStmt"));
  if (decl != nullptr) {
    decl->dump(os, _d + 1, LEAF, _p + "  ");
  }
}

void ReturnStmt::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{}\n", AST_STMT("ReturnStmt"));
  if (expr != nullptr) {
    expr->dump(os, _d + 1, LEAF, _p + "  ");
  }
}

/**
 * Decl
 */

void VarDecl::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}\n", AST_DECL("VarDecl"), AST_LITERAL("{}", name),
             AST_TYPE("'{}'", getType()));
  if (init != nullptr) {
    init->dump(os, _d + 1, LEAF, _p + "  ");
  }
}

void ParamVarDecl::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}\n", AST_DECL("ParamVarDecl"), AST_LITERAL("{}", name),
             AST_TYPE("'{}'", getType()));
}

void FunctionDecl::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}\n", AST_DECL("FunctionDecl"), AST_LITERAL("{}", name),
             AST_TYPE("'{}'", type));
  /// TODO: parameters declarations
  if (body != nullptr) {
    body->dump(os, _d + 1, LEAF, _p + "  ");
  }
}

/**
 * TranslationUnitDecl
 */

void TranslationUnitDecl::dump(std::ostream &os, size_t _d, Side _s,
                               std::string _p) {
  os << AST_DECL("TranslationUnitDecl") << "\n";
  if (decls.size() == 1) {
    decls[0]->dump(os, _d + 1, LEAF, _p);
  } else if (decls.size() >= 2) {
    for (int i = 0; i < decls.size() - 1; i++) {
      decls[i]->dump(os, _d + 1, INTERNAL, _p + "|");
    }
    decls[decls.size() - 1]->dump(os, _d + 1, LEAF, _p);
  }
}

} // namespace toyc
