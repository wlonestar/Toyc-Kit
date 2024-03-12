//! AST Print

#include <AST/AST.h>
#include <AST/ASTPrint.h>
#include <Util.h>

#include <cstddef>
#include <ostream>

namespace toyc {

void printASTLeader(std::ostream &os, size_t _d, Side _s, std::string _p) {
  std::string leader;
  if (_d == 0) {
    leader = "`";
  } else {
    if (_s == LEAF) {
      leader = _p + "`";
    } else {
      leader = _p;
    }
  }
  os << AST_LEADER("{}-", leader);
}

std::string attachLeafLeader(Side _s, std::string _p) {
  std::string leader = _p + " ";
  if (_s == LEAF) {
    leader += " ";
  }
  return leader;
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
  std::string declType = "(null)";
  if (dynamic_cast<VarDecl *>(decl.get())) {
    declType = "Var";
  } else if (dynamic_cast<FunctionDecl *>(decl.get())) {
    declType = "Function";
  }
  os << fstr("{} {} {} {}\n", AST_STMT("DeclRefExpr"),
             AST_TYPE("'{}'", decl->getType()), AST_DECL("{}", declType),
             AST_LITERAL("'{}'", decl->getName()));
}

void ImplicitCastExpr::dump(std::ostream &os, size_t _d, Side _s,
                            std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {}\n", AST_STMT("ImplicitCastExpr"), AST_TYPE("'{}'", type));
  std::string leader = attachLeafLeader(_s, _p);
  expr->dump(os, _d + 1, LEAF, leader);
}

void CastExpr::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {}\n", AST_STMT("CastExpr"), AST_TYPE("'{}'", type));
  std::string leader = attachLeafLeader(_s, _p);
  expr->dump(os, _d + 1, LEAF, leader);
}

void ParenExpr::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {}\n", AST_STMT("ParenExpr"), AST_TYPE("'{}'", getType()));
  std::string leader = attachLeafLeader(_s, _p);
  expr->dump(os, _d + 1, LEAF, leader);
}

void CallExpr::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {}\n", AST_STMT("CallExpr"), AST_TYPE("'{}'", getType()));
  std::string leader = attachLeafLeader(_s, _p);
  if (args.size() != 0) {
    callee->dump(os, _d + 1, INTERNAL, leader + "|");
  } else {
    callee->dump(os, _d + 1, LEAF, leader);
  }
  if (args.size() != 0) {
    for (size_t i = 0; i < args.size() - 1; i++) {
      args[i]->dump(os, _d + 1, INTERNAL, leader + "|");
    }
    args[args.size() - 1]->dump(os, _d + 1, LEAF, leader);
  }
}

void UnaryOperator::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {} '{}'\n", AST_STMT("UnaryOperator"),
             AST_TYPE("'{}'", type), side == PREFIX ? "prefix" : "postfix",
             op.value);
  std::string leader = attachLeafLeader(_s, _p);
  expr->dump(os, _d + 1, LEAF, leader);
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
  std::string leader = attachLeafLeader(_s, _p);
  if (stmts.size() == 1) {
    stmts[0]->dump(os, _d + 1, LEAF, leader);
  } else if (stmts.size() >= 2) {
    for (int i = 0; i < stmts.size() - 1; i++) {
      stmts[i]->dump(os, _d + 1, INTERNAL, leader + "|");
    }
    stmts[stmts.size() - 1]->dump(os, _d + 1, LEAF, leader);
  }
}

void ExprStmt::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{}\n", AST_STMT("ExprStmt"));
  std::string leader = attachLeafLeader(_s, _p);
  if (expr != nullptr) {
    expr->dump(os, _d + 1, LEAF, leader);
  }
}

void DeclStmt::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{}\n", AST_STMT("DeclStmt"));
  std::string leader = attachLeafLeader(_s, _p);
  if (decl != nullptr) {
    decl->dump(os, _d + 1, LEAF, leader);
  }
}

void IfStmt::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{}\n", AST_STMT("IfStmt"));
  std::string leader = attachLeafLeader(_s, _p);
  cond->dump(os, _d + 1, INTERNAL, leader + "|");
  if (elseStmt != nullptr) {
    thenStmt->dump(os, _d + 1, INTERNAL, leader + "|");
  } else {
    thenStmt->dump(os, _d + 1, LEAF, leader);
  }
  if (elseStmt != nullptr) {
    elseStmt->dump(os, _d + 1, LEAF, leader);
  }
}

void WhileStmt::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{}\n", AST_STMT("WhileStmt"));
  std::string leader = attachLeafLeader(_s, _p);
  cond->dump(os, _d + 1, INTERNAL, leader + "|");
  if (stmt != nullptr) {
    stmt->dump(os, _d + 1, LEAF, leader);
  }
}

void ForStmt::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{}\n", AST_STMT("ForStmt"));
  std::string leader = attachLeafLeader(_s, _p);
  init->dump(os, _d + 1, INTERNAL, leader + "|");
  cond->dump(os, _d + 1, INTERNAL, leader + "|");
  update->dump(os, _d + 1, INTERNAL, leader + "|");
  body->dump(os, _d + 1, LEAF, leader);
}

void ReturnStmt::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{}\n", AST_STMT("ReturnStmt"));
  std::string leader = attachLeafLeader(_s, _p);
  if (expr != nullptr) {
    expr->dump(os, _d + 1, LEAF, leader);
  }
}

/**
 * Decl
 */

void VarDecl::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}\n", AST_DECL("VarDecl"), AST_LITERAL("{}", name),
             AST_TYPE("'{}'", getType()));
  std::string leader = attachLeafLeader(_s, _p);
  if (init != nullptr) {
    init->dump(os, _d + 1, LEAF, leader);
  }
}

void ParmVarDecl::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}\n", AST_DECL("ParmVarDecl"), AST_LITERAL("{}", name),
             AST_TYPE("'{}'", getType()));
}

void FunctionDecl::dump(std::ostream &os, size_t _d, Side _s, std::string _p) {
  printASTLeader(os, _d, _s, _p);
  os << fstr("{} {} {}{}\n", AST_DECL("FunctionDecl"), AST_LITERAL("{}", proto->name),
             AST_TYPE("'{}'", proto->type), kind == EXTERN_FUNC ? " extern" : "");
  std::string leader = attachLeafLeader(_s, _p);
  for (size_t i = 0; i < proto->params.size(); i++) {
    if (i == proto->params.size() - 1 && body == nullptr) {
      proto->params[i]->dump(os, _d + 1, LEAF, leader);
    } else {
      proto->params[i]->dump(os, _d + 1, INTERNAL, leader + "|");
    }
  }
  if (body != nullptr) {
    body->dump(os, _d + 1, LEAF, leader);
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
