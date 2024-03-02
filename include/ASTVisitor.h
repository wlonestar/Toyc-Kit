//! code generation

#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#pragma once

#include <AST.h>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <map>

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
  virtual llvm::Value *codegen(const ReturnStmt &stmt) = 0;

  /**
   * Decl
   */

  virtual llvm::Value *codegen(const VarDecl &decl) = 0;
  virtual llvm::Type *codegen(const ParmVarDecl &decl) = 0;
  virtual llvm::Function *codegen(const FunctionDecl &decl) = 0;

  /**
   * TranslationUnitDecl
   */

  virtual void codegen(const TranslationUnitDecl &decl) = 0;
};

} // namespace toyc

#endif
