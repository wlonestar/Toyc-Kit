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

llvm::Value *IntegerLiteral::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *FloatingLiteral::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *CharacterLiteral::accept(ASTVisitor &visitor) { return nullptr; }

llvm::Value *StringLiteral::accept(ASTVisitor &visitor) { return nullptr; }

std::string DeclRefExpr::getType() const { return decl->getType(); }

llvm::Value *DeclRefExpr::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *ImplicitCastExpr::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *ParenExpr::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *CallExpr::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *UnaryOperator::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *BinaryOperator::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

/**
 * Stmt
 */

llvm::Value *CompoundStmt::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *ExprStmt::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *DeclStmt::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *IfStmt::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *WhileStmt::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *ForStmt::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Value *ReturnStmt::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

/**
 * Decl
 */

llvm::Value *VarDecl::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Type *ParmVarDecl::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

llvm::Function *FunctionDecl::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

} // namespace toyc
