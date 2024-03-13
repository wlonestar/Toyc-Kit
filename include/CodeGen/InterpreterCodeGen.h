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

namespace toyc {

class InterpreterIRVisitor : public BaseIRVisitor {
private:
  std::unique_ptr<ToycJIT> JIT;
  std::unique_ptr<llvm::legacy::FunctionPassManager> FPM;
  std::unique_ptr<llvm::LoopAnalysisManager> LAM;
  std::unique_ptr<llvm::FunctionAnalysisManager> FAM;
  std::unique_ptr<llvm::CGSCCAnalysisManager> CGAM;
  std::unique_ptr<llvm::ModuleAnalysisManager> MAM;
  std::unique_ptr<llvm::PassInstrumentationCallbacks> PIC;
  std::unique_ptr<llvm::StandardInstrumentations> SI;

  llvm::ExitOnError ExitOnErr;

  struct GlobalVar {
    std::string name;
    std::string type;
    size_t refered;
    GlobalVar(std::string _name, std::string _type, size_t _refered)
        : name(_name), type(_type), refered(_refered) {}
  };

  std::map<std::string, std::unique_ptr<GlobalVar>> globalVarEnv;
  std::map<std::string, std::unique_ptr<FunctionProto>> functionEnv;

private:
  void initialize();
  void resetReferGlobalVar();

public:
  InterpreterIRVisitor();

public:
  llvm::GlobalVariable *getGlobalVar(std::string name);

public:
  virtual llvm::Value *codegen(const DeclRefExpr &expr) override;
  virtual llvm::Value *codegen(const CallExpr &expr) override;
  virtual llvm::Value *codegen(const UnaryOperator &expr) override;
  virtual llvm::Value *codegen(const BinaryOperator &expr) override;

public:
  virtual llvm::Value *codegen(const VarDecl &decl) override;

public:
  void handleDeclaration(std::unique_ptr<Decl> &decl);
  void handleStatement(std::unique_ptr<Stmt> &stmt);
  void handleExpression(std::unique_ptr<Expr> &expr);
};

} // namespace toyc

#endif
