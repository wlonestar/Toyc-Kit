//! code generation

#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#pragma once

#include <AST/AST.h>

#include <llvm/IR/Value.h>

namespace toyc {

class ASTVisitor {
public:
  /**
   * Expr
   */

  virtual llvm::Value *codegen(const IntegerLiteral &expr) = 0;
  virtual llvm::Value *codegen(const FloatingLiteral &expr) = 0;
  virtual llvm::Value *codegen(const DeclRefExpr &expr) = 0;
  virtual llvm::Value *codegen(const ImplicitCastExpr &expr) = 0;
  virtual llvm::Value *codegen(const CastExpr &expr) = 0;
  virtual llvm::Value *codegen(const ParenExpr &expr) = 0;
  virtual llvm::Value *codegen(const CallExpr &expr) = 0;
  virtual llvm::Value *codegen(const UnaryOperator &expr) = 0;
  virtual llvm::Value *codegen(const BinaryOperator &expr) = 0;

  /**
   * Stmt
   */

  virtual llvm::Value *codegen(const CompoundStmt &stmt) = 0;
  virtual llvm::Value *codegen(const ExprStmt &stmt) = 0;
  virtual llvm::Value *codegen(const DeclStmt &stmt) = 0;
  virtual llvm::Value *codegen(const IfStmt &stmt) = 0;
  virtual llvm::Value *codegen(const WhileStmt &stmt) = 0;
  virtual llvm::Value *codegen(const ForStmt &stmt) = 0;
  virtual llvm::Value *codegen(const ReturnStmt &stmt) = 0;

  /**
   * Decl
   */

  virtual llvm::Value *codegen(const VarDecl &decl) = 0;
  virtual llvm::Type *codegen(const ParmVarDecl &decl) = 0;
  virtual llvm::Function *codegen(const FunctionDecl &decl) = 0;

  // /**
  //  * TranslationUnitDecl
  //  */

  // virtual void codegen(const TranslationUnitDecl &decl) = 0;
};

} // namespace toyc

#endif
