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
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

#include <map>

namespace toyc {

class CodeGenException : public std::exception {
private:
  std::string message;

public:
  CodeGenException(std::string &_message)
      : message(fstr("\033[1;31merror:\033[0m \033[1;37m{}\033[0m", _message)) {
  }

  CodeGenException(std::string &&_message)
      : message(fstr("\033[1;31merror:\033[0m \033[1;37m{}\033[0m", _message)) {
  }

  const char *what() const noexcept override { return message.c_str(); }
};

class IRCodegenVisitor : public ASTVisitor {
protected:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::map<std::string, llvm::GlobalVariable *> globalVarEnv;
  std::map<std::string, llvm::AllocaInst *> varEnv;

private:
  void printGlobalVarEnv() {
    std::cout << "\033[1;32mGlobalVarEnv:\n";
    for (auto &[first, second] : globalVarEnv) {
      std::string valueStr;
      llvm::raw_string_ostream ros(valueStr);
      if (second != nullptr) {
        second->print(ros);
      } else {
        ros << "<null>";
      }
      std::cout << fstr("  <var> '{}': {}\n", first, valueStr);
    }
    std::cout << "\033[0m\n";
  }

  void printVarEnv() {
    std::cout << "\033[1;32mVarEnv:\n";
    for (auto &[first, second] : varEnv) {
      std::string valueStr;
      llvm::raw_string_ostream ros(valueStr);
      if (second != nullptr) {
        second->print(ros);
      } else {
        ros << "<null>";
      }
      std::cout << fstr("  <var> '{}': {}\n", first, valueStr);
    }
    std::cout << "\033[0m\n";
  }

  void clearVarEnv() { varEnv.clear(); }

public:
  IRCodegenVisitor();

  void dump(llvm::raw_ostream &os = llvm::errs());
  bool verifyModule(llvm::raw_ostream &os = llvm::errs());

public:
  llvm::Function *codegenFuncTy(const FunctionDecl &decl);

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
