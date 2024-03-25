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
  CodeGenException(std::string _msg)
      : message(makeString("\033[1;31merror:\033[0m \033[1;37m{}\033[0m", _msg)) {}

  const char *what() const noexcept override { return message.c_str(); }
};

/**
 * Base IR visitor
 */

class BaseIRVisitor : public ASTVisitor {
protected:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;

protected:
  /// local variable table
  std::map<std::string, llvm::AllocaInst *> varEnv;

protected:
  void printVarEnv();
  void clearVarEnv() { varEnv.clear(); }

public:
  BaseIRVisitor() {}

public:
  virtual void dump(llvm::raw_ostream &os = llvm::errs());
  virtual bool verifyModule(llvm::raw_ostream &os = llvm::errs());

public:
  virtual llvm::Function *getFunction(const FunctionDecl &decl);

public:
  virtual llvm::Value *codegen(const IntegerLiteral &expr) override;
  virtual llvm::Value *codegen(const FloatingLiteral &expr) override;
  virtual llvm::Value *codegen(const ImplicitCastExpr &expr) override;
  virtual llvm::Value *codegen(const ParenExpr &expr) override;

public:
  virtual llvm::Value *codegen(const CompoundStmt &stmt) override;
  virtual llvm::Value *codegen(const ExprStmt &stmt) override;
  virtual llvm::Value *codegen(const DeclStmt &stmt) override;
  virtual llvm::Value *codegen(const IfStmt &stmt) override;
  virtual llvm::Value *codegen(const WhileStmt &stmt) override;
  virtual llvm::Value *codegen(const ForStmt &stmt) override;
  virtual llvm::Value *codegen(const ReturnStmt &stmt) override;

public:
  virtual llvm::Type *codegen(const ParmVarDecl &decl) override;
  virtual llvm::Function *codegen(const FunctionDecl &decl) override;
};

class CompilerIRVisitor : public BaseIRVisitor {
private:
  /// global variable table
  std::map<std::string, llvm::GlobalVariable *> globalVarEnv;

private:
  void printGlobalVarEnv();

public:
  CompilerIRVisitor();

public:
  void setModuleID(std::string &name);

public:
  virtual llvm::Value *codegen(const DeclRefExpr &expr) override;
  virtual llvm::Value *codegen(const CallExpr &expr) override;
  virtual llvm::Value *codegen(const UnaryOperator &expr) override;
  virtual llvm::Value *codegen(const BinaryOperator &expr) override;

public:
  virtual llvm::Value *codegen(const VarDecl &decl) override;

public:
  void codegen(const TranslationUnitDecl &decl);
};

} // namespace toyc

#endif
