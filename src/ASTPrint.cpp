//! AST Print

#include <AST.h>
#include <ASTPrint.h>
#include <Util.h>

#include <cstddef>
#include <iostream>

namespace toyc {

void printASTLeader(size_t _d, Side _s, std::string _p) {
  std::cout << AST_LEADER("{}-",
                          (_d == 0 ? "`" : (_s == LEAF ? _p + "`" : _p)));
}

/**
 * Expr
 */

void IntegerLiteral::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} {} {}\n", AST_STMT("IntegerLiteral"),
                    AST_TYPE("'{}'", type), AST_LITERAL("{}", value));
}

void FloatingLiteral::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} {} {}\n", AST_STMT("FloatingLiteral"),
                    AST_TYPE("'{}'", type), AST_LITERAL("{}", value));
}

void CharacterLiteral::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} {} {}\n", AST_STMT("CharacterLiteral"),
                    AST_TYPE("'{}'", getType()), AST_LITERAL("{}", value));
}

void StringLiteral::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} {} {}\n", AST_STMT("StringLiteral"),
                    AST_TYPE("'{}'", getType()), AST_LITERAL("\"{}\"", value));
}

void DeclRefExpr::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} {} {}\n", AST_STMT("DeclRefExpr"),
                    AST_TYPE("'{}'", getType()), AST_LITERAL("'{}'", name));
}

void ParenExpr::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} {}\n", AST_STMT("ParenExpr"),
                    AST_TYPE("'{}'", getType()));
  expr->dump(_d + 1, LEAF, _p + "  ");
}

void UnaryOperator::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} '{}'\n", AST_STMT("UnaryOperator"), op.value);
  right->dump(_d + 1, LEAF, _p + "  ");
}

void BinaryOperator::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} {} '{}'\n", AST_STMT("BinaryOperator"),
                    AST_TYPE("'{}'", getType()), op.value);
  left->dump(_d + 1, INTERNAL, _p + "  |");
  right->dump(_d + 1, LEAF, _p + "  ");
}

/**
 * Stmt
 */

void CompoundStmt::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{}\n", AST_STMT("CompoundStmt"));
  if (stmts.size() == 1) {
    stmts[0]->dump(_d + 1, LEAF, _p + "  ");
  } else if (stmts.size() >= 2) {
    for (int i = 0; i < stmts.size() - 1; i++) {
      stmts[i]->dump(_d + 1, INTERNAL, _p + "  |");
    }
    stmts[stmts.size() - 1]->dump(_d + 1, LEAF, _p + "  ");
  }
}

void ExprStmt::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{}\n", AST_STMT("ExprStmt"));
  if (expr != nullptr) {
    expr->dump(_d + 1, LEAF, _p + "  ");
  }
}

void DeclStmt::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{}\n", AST_STMT("DeclStmt"));
  if (decl != nullptr) {
    decl->dump(_d + 1, LEAF, _p + "  ");
  }
}

void ReturnStmt::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{}\n", AST_STMT("ReturnStmt"));
  if (expr != nullptr) {
    expr->dump(_d + 1, LEAF, _p + "  ");
  }
}

/**
 * Decl
 */

void VarDecl::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} {} {}\n", AST_DECL("VarDecl"), AST_LITERAL("{}", name),
                    AST_TYPE("'{}'", getType()));
  if (init != nullptr) {
    init->dump(_d + 1, LEAF, _p + "  ");
  }
}

void ParamVarDecl::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} {} {}\n", AST_DECL("ParamVarDecl"),
                    AST_LITERAL("{}", name), AST_TYPE("'{}'", getType()));
}

void FunctionDecl::dump(size_t _d, Side _s, std::string _p) {
  printASTLeader(_d, _s, _p);
  std::cout << fstr("{} {} {}\n", AST_DECL("FunctionDecl"),
                    AST_LITERAL("{}", name), AST_TYPE("'{}'", type));
  /// TODO: parameters declarations
  if (body != nullptr) {
    body->dump(_d + 1, LEAF, _p + "  ");
  }
}

/**
 * TranslationUnitDecl
 */

void TranslationUnitDecl::dump(size_t _d, Side _s, std::string _p) {
  std::cout << AST_DECL("TranslationUnitDecl") << "\n";
  if (decls.size() == 1) {
    decls[0]->dump(_d + 1, LEAF, _p);
  } else if (decls.size() >= 2) {
    for (int i = 0; i < decls.size() - 1; i++) {
      decls[i]->dump(_d + 1, INTERNAL, _p + "|");
    }
    decls[decls.size() - 1]->dump(_d + 1, LEAF, _p);
  }
}

} // namespace toyc
