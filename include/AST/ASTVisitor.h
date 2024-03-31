//! code generation

#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#pragma once

#include <AST/AST.h>

#include <llvm/IR/Value.h>

namespace toyc {

class ASTVisitor {
public:
  virtual auto Codegen(const IntegerLiteral &expr) -> llvm::Value * = 0;
  virtual auto Codegen(const FloatingLiteral &expr) -> llvm::Value * = 0;
  virtual auto Codegen(const DeclRefExpr &expr) -> llvm::Value * = 0;
  virtual auto Codegen(const ImplicitCastExpr &expr) -> llvm::Value * = 0;
  virtual auto Codegen(const ParenExpr &expr) -> llvm::Value * = 0;
  virtual auto Codegen(const CallExpr &expr) -> llvm::Value * = 0;
  virtual auto Codegen(const UnaryOperator &expr) -> llvm::Value * = 0;
  virtual auto Codegen(const BinaryOperator &expr) -> llvm::Value * = 0;

  virtual auto Codegen(const CompoundStmt &stmt) -> llvm::Value * = 0;
  virtual auto Codegen(const ExprStmt &stmt) -> llvm::Value * = 0;
  virtual auto Codegen(const DeclStmt &stmt) -> llvm::Value * = 0;
  virtual auto Codegen(const IfStmt &stmt) -> llvm::Value * = 0;
  virtual auto Codegen(const WhileStmt &stmt) -> llvm::Value * = 0;
  virtual auto Codegen(const ForStmt &stmt) -> llvm::Value * = 0;
  virtual auto Codegen(const ReturnStmt &stmt) -> llvm::Value * = 0;

  virtual auto Codegen(const VarDecl &decl) -> llvm::Value * = 0;
  virtual auto Codegen(const ParmVarDecl &decl) -> llvm::Type * = 0;
  virtual auto Codegen(const FunctionDecl &decl) -> llvm::Function * = 0;
};

} // namespace toyc

#endif
