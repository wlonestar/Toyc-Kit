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

/**
 * @brief Codegen Visitor for interpreter
 *
 */
class InterpreterIRCodegenVisitor : public ASTVisitor {
private:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
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
    GlobalVar(std::string _name, std::string _type)
        : name(_name), type(_type) {}
  };
  std::map<std::string, std::unique_ptr<GlobalVar>> globalVarEnv;
  std::map<std::string, llvm::AllocaInst *> varEnv;

  /// store function prototype
  std::map<std::string, std::unique_ptr<FunctionProto>> functionEnv;

private:
  void initialize();
  void clearVarEnv() { varEnv.clear(); }

public:
  InterpreterIRCodegenVisitor();

  virtual void dump(llvm::raw_ostream &os = llvm::errs());
  virtual bool verifyModule(llvm::raw_ostream &os = llvm::errs());

public:
  llvm::GlobalVariable *getGlobalVar(std::string name);
  llvm::Function *getFunction(const FunctionDecl &proto);

public:
  /**
   * Expr
   */

  virtual llvm::Value *codegen(const IntegerLiteral &expr) override;
  virtual llvm::Value *codegen(const FloatingLiteral &expr) override;
  virtual llvm::Value *codegen(const DeclRefExpr &expr) override;
  virtual llvm::Value *codegen(const ImplicitCastExpr &expr) override;
  virtual llvm::Value *codegen(const CastExpr &expr) override;
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

  void handleDeclaration(std::unique_ptr<Decl> &decl);
  void handleExpression(std::unique_ptr<Expr> &expr);
};

} // namespace toyc

#endif
