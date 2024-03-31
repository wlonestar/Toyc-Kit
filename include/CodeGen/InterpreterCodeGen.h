//! code generation

#ifndef INTERPRETER_CODE_GEN_H
#define INTERPRETER_CODE_GEN_H

#pragma once

#include <AST/AST.h>
#include <AST/ASTVisitor.h>
#include <CodeGen/CodeGen.h>
#include <Interpreter/JIT.h>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Value.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/Error.h>

#include <memory>
#include <utility>

namespace toyc {

class InterpreterIRVisitor : public BaseIRVisitor {
private:
  std::unique_ptr<ToycJIT> jit_;
  std::unique_ptr<llvm::legacy::FunctionPassManager> fpm_;
  std::unique_ptr<llvm::LoopAnalysisManager> lam_;
  std::unique_ptr<llvm::FunctionAnalysisManager> fam_;
  std::unique_ptr<llvm::CGSCCAnalysisManager> cgam_;
  std::unique_ptr<llvm::ModuleAnalysisManager> mam_;
  std::unique_ptr<llvm::PassInstrumentationCallbacks> pic_;
  std::unique_ptr<llvm::StandardInstrumentations> si_;

  llvm::ExitOnError exit_on_err_;

  struct GlobalVar {
    std::string name_;
    std::string type_;
    size_t refered_;
    GlobalVar(std::string _name, std::string _type, size_t _refered)
        : name_(std::move(_name)), type_(std::move(_type)), refered_(_refered) {
    }
  };

  std::map<std::string, std::unique_ptr<GlobalVar>> global_var_env_;
  std::map<std::string, std::unique_ptr<FunctionProto>> function_env_;

private:
  void Initialize();
  void ResetReferGlobalVar();
  void ResetReferFunctionProto();

public:
  InterpreterIRVisitor();

public:
  auto GetGlobalVar(const std::string &name) -> llvm::GlobalVariable *;
  auto GetFunction(const FunctionDecl &decl) -> llvm::Function * override;

public:
  auto Codegen(const DeclRefExpr &expr) -> llvm::Value * override;
  auto Codegen(const CallExpr &expr) -> llvm::Value * override;
  auto Codegen(const UnaryOperator &expr) -> llvm::Value * override;
  auto Codegen(const BinaryOperator &expr) -> llvm::Value * override;

public:
  auto Codegen(const VarDecl &decl) -> llvm::Value * override;

public:
  void HandleDeclaration(std::unique_ptr<Decl> &decl);
  void HandleStatement(std::unique_ptr<Stmt> &stmt);
  void HandleExpression(std::unique_ptr<Expr> &expr);
};

} // namespace toyc

#endif
