//! AST implementation

#include <AST.h>
#include <ASTVisitor.h>
#include <CodeGen.h>

#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace toyc {

/**
 * Expr
 */

std::string IntegerLiteral::getType() const { return type; }

llvm::Value *IntegerLiteral::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

std::string FloatingLiteral::getType() const { return type; }

llvm::Value *FloatingLiteral::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

std::string CharacterLiteral::getType() const { return "i64"; }

llvm::Value *CharacterLiteral::accept(ASTVisitor &visitor) {
  throw CodeGenException("not implemented!");
}

std::string StringLiteral::getType() const { return type; }

llvm::Value *StringLiteral::accept(ASTVisitor &visitor) {
  throw CodeGenException("not implemented!");
}

std::string DeclRefExpr::getType() const { return decl->getType(); }

llvm::Value *DeclRefExpr::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

std::string ImplicitCastExpr::getType() const { return type; }

llvm::Value *ImplicitCastExpr::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

std::string ParenExpr::getType() const { return expr->getType(); }

llvm::Value *ParenExpr::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

std::string CallExpr::getType() const { return callee->getType(); }

llvm::Value *CallExpr::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

std::string UnaryOperator::getType() const { return type; }

llvm::Value *UnaryOperator::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

std::string BinaryOperator::getType() const { return type; }

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

llvm::Value *ReturnStmt::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

/**
 * Decl
 */

std::string VarDecl::getName() const { return name; }

std::string VarDecl::getType() const { return type; }

llvm::Value *VarDecl::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

std::string ParmVarDecl::getName() const { return name; }

std::string ParmVarDecl::getType() const { return type; }

llvm::Type *ParmVarDecl::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

std::string FunctionDecl::getName() const { return name; }

std::string FunctionDecl::getType() const { return type; }

llvm::Function *FunctionDecl::accept(ASTVisitor &visitor) {
  return visitor.codegen(*this);
}

/**
 * TranslationUnitDecl
 */

void TranslationUnitDecl::accept(ASTVisitor &visitor) {
  visitor.codegen(*this);
}

} // namespace toyc
