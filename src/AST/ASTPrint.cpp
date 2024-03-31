//! AST Print

#include <AST/AST.h>
#include <AST/ASTPrint.h>
#include <Util.h>

#include <cstddef>
#include <ostream>

namespace toyc {

void PrintASTLeader(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
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

auto AttachLeafLeader(Side _s, const std::string &_p) -> std::string {
  std::string leader = _p + " ";
  if (_s == LEAF) {
    leader += " ";
  }
  return leader;
}

/**
 * Expr
 */

void IntegerLiteral::Dump(std::ostream &os, size_t _d, Side _s,
                          const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {} {}\n", AST_STMT("IntegerLiteral"),
                   AST_TYPE("'{}'", type_), AST_LITERAL("{}", value_));
}

void FloatingLiteral::Dump(std::ostream &os, size_t _d, Side _s,
                           const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {} {}\n", AST_STMT("FloatingLiteral"),
                   AST_TYPE("'{}'", type_), AST_LITERAL("{}", value_));
}

void CharacterLiteral::Dump(std::ostream &os, size_t _d, Side _s,
                            const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {} {}\n", AST_STMT("CharacterLiteral"),
                   AST_TYPE("'{}'", type_), AST_LITERAL("{}", value_));
}

void StringLiteral::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {} {}\n", AST_STMT("StringLiteral"),
                   AST_TYPE("'{}'", type_), AST_LITERAL("\"{}\"", value_));
}

void DeclRefExpr::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  std::string decl_type = "(null)";
  if (dynamic_cast<VarDecl *>(decl_.get()) != nullptr) {
    decl_type = "Var";
  } else if (dynamic_cast<FunctionDecl *>(decl_.get()) != nullptr) {
    decl_type = "Function";
  }
  os << makeString("{} {} {} {}\n", AST_STMT("DeclRefExpr"),
                   AST_TYPE("'{}'", decl_->GetType()), AST_DECL("{}", decl_type),
                   AST_LITERAL("'{}'", decl_->GetName()));
}

void ImplicitCastExpr::Dump(std::ostream &os, size_t _d, Side _s,
                            const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {}\n", AST_STMT("ImplicitCastExpr"),
                   AST_TYPE("'{}'", type_));
  std::string leader = AttachLeafLeader(_s, _p);
  expr_->Dump(os, _d + 1, LEAF, leader);
}

void ParenExpr::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {}\n", AST_STMT("ParenExpr"),
                   AST_TYPE("'{}'", GetType()));
  std::string leader = AttachLeafLeader(_s, _p);
  expr_->Dump(os, _d + 1, LEAF, leader);
}

void CallExpr::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {}\n", AST_STMT("CallExpr"),
                   AST_TYPE("'{}'", GetType()));
  std::string leader = AttachLeafLeader(_s, _p);
  if (!args_.empty()) {
    callee_->Dump(os, _d + 1, INTERNAL, leader + "|");
  } else {
    callee_->Dump(os, _d + 1, LEAF, leader);
  }
  if (!args_.empty()) {
    for (size_t i = 0; i < args_.size() - 1; i++) {
      args_[i]->Dump(os, _d + 1, INTERNAL, leader + "|");
    }
    args_[args_.size() - 1]->Dump(os, _d + 1, LEAF, leader);
  }
}

void UnaryOperator::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {} {} '{}'\n", AST_STMT("UnaryOperator"),
                   AST_TYPE("'{}'", type_),
                   side_ == PREFIX ? "prefix" : "postfix", op_.value_);
  std::string leader = AttachLeafLeader(_s, _p);
  expr_->Dump(os, _d + 1, LEAF, leader);
}

void BinaryOperator::Dump(std::ostream &os, size_t _d, Side _s,
                          const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {} '{}'\n", AST_STMT("BinaryOperator"),
                   AST_TYPE("'{}'", type_), op_.value_);
  left_->Dump(os, _d + 1, INTERNAL, _p + "  |");
  right_->Dump(os, _d + 1, LEAF, _p + "  ");
}

/**
 * Stmt
 */

void CompoundStmt::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{}\n", AST_STMT("CompoundStmt"));
  std::string leader = AttachLeafLeader(_s, _p);
  if (stmts_.size() == 1) {
    stmts_[0]->Dump(os, _d + 1, LEAF, leader);
  } else if (stmts_.size() >= 2) {
    for (int i = 0; i < stmts_.size() - 1; i++) {
      stmts_[i]->Dump(os, _d + 1, INTERNAL, leader + "|");
    }
    stmts_[stmts_.size() - 1]->Dump(os, _d + 1, LEAF, leader);
  }
}

void ExprStmt::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{}\n", AST_STMT("ExprStmt"));
  std::string leader = AttachLeafLeader(_s, _p);
  if (expr_ != nullptr) {
    expr_->Dump(os, _d + 1, LEAF, leader);
  }
}

void DeclStmt::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{}\n", AST_STMT("DeclStmt"));
  std::string leader = AttachLeafLeader(_s, _p);
  if (decl_ != nullptr) {
    decl_->Dump(os, _d + 1, LEAF, leader);
  }
}

void IfStmt::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{}\n", AST_STMT("IfStmt"));
  std::string leader = AttachLeafLeader(_s, _p);
  cond_->Dump(os, _d + 1, INTERNAL, leader + "|");
  if (else_stmt_ != nullptr) {
    then_stmt_->Dump(os, _d + 1, INTERNAL, leader + "|");
  } else {
    then_stmt_->Dump(os, _d + 1, LEAF, leader);
  }
  if (else_stmt_ != nullptr) {
    else_stmt_->Dump(os, _d + 1, LEAF, leader);
  }
}

void WhileStmt::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{}\n", AST_STMT("WhileStmt"));
  std::string leader = AttachLeafLeader(_s, _p);
  cond_->Dump(os, _d + 1, INTERNAL, leader + "|");
  if (stmt_ != nullptr) {
    stmt_->Dump(os, _d + 1, LEAF, leader);
  }
}

void ForStmt::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{}\n", AST_STMT("ForStmt"));
  std::string leader = AttachLeafLeader(_s, _p);
  init_->Dump(os, _d + 1, INTERNAL, leader + "|");
  cond_->Dump(os, _d + 1, INTERNAL, leader + "|");
  update_->Dump(os, _d + 1, INTERNAL, leader + "|");
  body_->Dump(os, _d + 1, LEAF, leader);
}

void ReturnStmt::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{}\n", AST_STMT("ReturnStmt"));
  std::string leader = AttachLeafLeader(_s, _p);
  if (expr_ != nullptr) {
    expr_->Dump(os, _d + 1, LEAF, leader);
  }
}

/**
 * Decl
 */

void VarDecl::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {} {}\n", AST_DECL("VarDecl"), AST_LITERAL("{}", name_),
                   AST_TYPE("'{}'", GetType()));
  std::string leader = AttachLeafLeader(_s, _p);
  if (init_ != nullptr) {
    init_->Dump(os, _d + 1, LEAF, leader);
  }
}

void ParmVarDecl::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {} {}\n", AST_DECL("ParmVarDecl"),
                   AST_LITERAL("{}", name_), AST_TYPE("'{}'", GetType()));
}

void FunctionDecl::Dump(std::ostream &os, size_t _d, Side _s, const std::string &_p) {
  PrintASTLeader(os, _d, _s, _p);
  os << makeString("{} {} {}{}\n", AST_DECL("FunctionDecl"),
                   AST_LITERAL("{}", proto_->name_),
                   AST_TYPE("'{}'", proto_->type_),
                   kind_ == EXTERN_FUNC ? " extern" : "");
  std::string leader = AttachLeafLeader(_s, _p);
  for (size_t i = 0; i < proto_->params_.size(); i++) {
    if (i == proto_->params_.size() - 1 && body_ == nullptr) {
      proto_->params_[i]->Dump(os, _d + 1, LEAF, leader);
    } else {
      proto_->params_[i]->Dump(os, _d + 1, INTERNAL, leader + "|");
    }
  }
  if (body_ != nullptr) {
    body_->Dump(os, _d + 1, LEAF, leader);
  }
}

/**
 * TranslationUnitDecl
 */

void TranslationUnitDecl::Dump(std::ostream &os, size_t _d, Side _s,
                               const std::string &_p) {
  os << AST_DECL("TranslationUnitDecl") << "\n";
  if (decls_.size() == 1) {
    decls_[0]->Dump(os, _d + 1, LEAF, _p);
  } else if (decls_.size() >= 2) {
    for (int i = 0; i < decls_.size() - 1; i++) {
      decls_[i]->Dump(os, _d + 1, INTERNAL, _p + "|");
    }
    decls_[decls_.size() - 1]->Dump(os, _d + 1, LEAF, _p);
  }
}

} // namespace toyc
