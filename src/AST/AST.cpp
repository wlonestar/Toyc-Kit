//! AST implementation

#include <AST/AST.h>
#include <AST/ASTVisitor.h>

#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace toyc {

/**
 * Expr
 */

auto IntegerLiteral::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto FloatingLiteral::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto StringLiteral::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return nullptr;
}

auto DeclRefExpr::GetType() const -> std::string { return decl_->GetType(); }

auto DeclRefExpr::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto ImplicitCastExpr::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto ParenExpr::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto CallExpr::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto UnaryOperator::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto BinaryOperator::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

/**
 * Stmt
 */

auto CompoundStmt::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto ExprStmt::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto DeclStmt::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto IfStmt::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto WhileStmt::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto ForStmt::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto ReturnStmt::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

/**
 * Decl
 */

auto VarDecl::Accept(ASTVisitor &visitor) -> llvm::Value * {
  return visitor.Codegen(*this);
}

auto ParmVarDecl::Accept(ASTVisitor &visitor) -> llvm::Type * {
  return visitor.Codegen(*this);
}

auto FunctionDecl::Accept(ASTVisitor &visitor) -> llvm::Function * {
  return visitor.Codegen(*this);
}

} // namespace toyc
