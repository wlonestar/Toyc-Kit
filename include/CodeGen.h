//! code generation

#ifndef CODE_GEN_H
#define CODE_GEN_H

#pragma once

#include <AST.h>
#include <ASTVisitor.h>

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

class IRCodegenVisitor : public ASTVisitor {
protected:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;

public:
  IRCodegenVisitor();

  void modulePrint(llvm::raw_ostream &OS, llvm::AssemblyAnnotationWriter *AAW,
                   bool ShouldPreserveUseListOrder = false,
                   bool IsForDebug = false) const;

  bool verifyModule(llvm::raw_ostream *OS = nullptr,
                    bool *BrokenDebugInfo = nullptr);

public:
  /**
   * Expr
   */

  virtual llvm::Value *codegen(const IntegerLiteral &expr) override;
  virtual llvm::Value *codegen(const FloatingLiteral &expr) override;
  virtual llvm::Value *codegen(const DeclRefExpr &expr) override;
  virtual llvm::Value *codegen(const ParenExpr &expr) override;
  virtual llvm::Value *codegen(const UnaryOperator &expr) override;
  virtual llvm::Value *codegen(const BinaryOperator &expr) override;

  /**
   * Stmt
   */

  virtual llvm::Value *codegen(const CompoundStmt &stmt) override;
  virtual llvm::Value *codegen(const ExprStmt &stmt) override;
  virtual llvm::Value *codegen(const DeclStmt &stmt) override;
  virtual llvm::Value *codegen(const ReturnStmt &stmt) override;

  /**
   * Decl
   */

  virtual llvm::Value *codegen(const VarDecl &decl) override;
  virtual llvm::Value *codegen(const ParamVarDecl &decl) override;
  virtual llvm::Function *codegen(const FunctionDecl &decl) override;

  /**
   * TranslationUnitDecl
   */

  virtual void codegen(const TranslationUnitDecl &decl) override;
};

} // namespace toyc

#endif
