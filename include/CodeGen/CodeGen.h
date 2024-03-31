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
  std::string message_;

public:
  explicit CodeGenException(std::string _msg)
      : message_(
            makeString("\033[1;31merror:\033[0m \033[1;37m{}\033[0m", _msg)) {}

  auto what() const noexcept -> const char * override {
    return message_.c_str();
  }
};

/**
 * Base IR visitor
 */

class BaseIRVisitor : public ASTVisitor {
protected:
  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  std::unique_ptr<llvm::IRBuilder<>> builder_;

protected:
  /// local variable table
  std::map<std::string, llvm::AllocaInst *> var_env_;

protected:
  void PrintVarEnv();
  void ClearVarEnv() { var_env_.clear(); }

public:
  BaseIRVisitor() = default;

public:
  virtual void Dump(llvm::raw_ostream &os = llvm::errs());
  virtual auto VerifyModule(llvm::raw_ostream &os = llvm::errs()) -> bool;

public:
  virtual auto GetFunction(const FunctionDecl &decl) -> llvm::Function *;

public:
  auto Codegen(const IntegerLiteral &expr) -> llvm::Value * override;
  auto Codegen(const FloatingLiteral &expr) -> llvm::Value * override;
  auto Codegen(const ImplicitCastExpr &expr) -> llvm::Value * override;
  auto Codegen(const ParenExpr &expr) -> llvm::Value * override;

public:
  auto Codegen(const CompoundStmt &stmt) -> llvm::Value * override;
  auto Codegen(const ExprStmt &stmt) -> llvm::Value * override;
  auto Codegen(const DeclStmt &stmt) -> llvm::Value * override;
  auto Codegen(const IfStmt &stmt) -> llvm::Value * override;
  auto Codegen(const WhileStmt &stmt) -> llvm::Value * override;
  auto Codegen(const ForStmt &stmt) -> llvm::Value * override;
  auto Codegen(const ReturnStmt &stmt) -> llvm::Value * override;

public:
  auto Codegen(const ParmVarDecl &decl) -> llvm::Type * override;
  auto Codegen(const FunctionDecl &decl) -> llvm::Function * override;
};

class CompilerIRVisitor : public BaseIRVisitor {
private:
  /// global variable table
  std::map<std::string, llvm::GlobalVariable *> global_var_env_;

private:
  void PrintGlobalVarEnv();

public:
  CompilerIRVisitor();

public:
  void SetModuleID(std::string &name);

public:
  auto Codegen(const DeclRefExpr &expr) -> llvm::Value * override;
  auto Codegen(const CallExpr &expr) -> llvm::Value * override;
  auto Codegen(const UnaryOperator &expr) -> llvm::Value * override;
  auto Codegen(const BinaryOperator &expr) -> llvm::Value * override;

public:
  auto Codegen(const VarDecl &decl) -> llvm::Value * override;

public:
  void Codegen(const TranslationUnitDecl &decl);
};

} // namespace toyc

#endif
