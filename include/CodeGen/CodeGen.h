//! code generation

#ifndef CODE_GEN_H
#define CODE_GEN_H

#pragma once

#include <AST/AST.h>
#include <AST/ASTVisitor.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

#include <map>

namespace toyc {

class CodeGenException : public std::exception {
private:
  std::string message;

public:
  CodeGenException(std::string &_msg)
      : message(fstr("\033[1;31merror:\033[0m \033[1;37m{}\033[0m", _msg)) {}
  CodeGenException(std::string &&_msg)
      : message(fstr("\033[1;31merror:\033[0m \033[1;37m{}\033[0m", _msg)) {}

  const char *what() const noexcept override { return message.c_str(); }
};

/**
 * @brief Codegen Visitor for compiler
 *
 */
class CompilerCodegenVisitor : public ASTVisitor {
protected:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::map<std::string, llvm::GlobalVariable *> globalVarEnv;
  std::map<std::string, llvm::AllocaInst *> varEnv;

protected:
  void printGlobalVarEnv();
  void printVarEnv();
  void clearVarEnv() { varEnv.clear(); }

public:
  CompilerCodegenVisitor();

  virtual void dump(llvm::raw_ostream &os = llvm::errs());
  virtual bool verifyModule(llvm::raw_ostream &os = llvm::errs());
  virtual void setModuleID(std::string &name);

public:
  virtual llvm::Function *codegenFuncTy(const FunctionDecl &decl);

public:
  /**
   * Expr
   */

  virtual llvm::Value *codegen(const IntegerLiteral &expr) override;
  virtual llvm::Value *codegen(const FloatingLiteral &expr) override;
  virtual llvm::Value *codegen(const DeclRefExpr &expr) override;
  virtual llvm::Value *codegen(const ImplicitCastExpr &expr) override;
  virtual llvm::Value *codegen(const ParenExpr &expr) override;
  virtual llvm::Value *codegen(const CallExpr &expr) override;
  virtual llvm::Value *codegen(const UnaryOperator &expr) override;
  virtual llvm::Value *codegen(const BinaryOperator &expr) override;

  /**
   * Stmt
   */

  virtual llvm::Value *codegen(const CompoundStmt &stmt) override;
  virtual llvm::Value *codegen(const ExprStmt &stmt) override;
  virtual llvm::Value *codegen(const DeclStmt &stmt) override;
  virtual llvm::Value *codegen(const IfStmt &stmt) override;
  virtual llvm::Value *codegen(const WhileStmt &stmt) override;
  virtual llvm::Value *codegen(const ForStmt &stmt) override;
  virtual llvm::Value *codegen(const ReturnStmt &stmt) override;

  /**
   * Decl
   */

  virtual llvm::Value *codegen(const VarDecl &decl) override;
  virtual llvm::Type *codegen(const ParmVarDecl &decl) override;
  virtual llvm::Function *codegen(const FunctionDecl &decl) override;

  /**
   * TranslationUnitDecl
   */

  virtual void codegen(const TranslationUnitDecl &decl) override;
};

} // namespace toyc

#endif
