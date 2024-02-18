// ! code generation implementation

#include <AST.h>
#include <CodeGen.h>
#include <Parser.h>
#include <Token.h>
#include <Util.h>

#include <exception>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <memory>
#include <utility>
#include <vector>

namespace toyc {

std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;

void initializeModule() {
  /// Open a new context and module.
  TheContext = std::make_unique<llvm::LLVMContext>();
  TheModule = std::make_unique<llvm::Module>("toyc jit", *TheContext);
  /// Set data layout and target triple explicitly
  TheModule->setDataLayout(
      "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128");
  TheModule->setTargetTriple("x86_64-pc-linux-gnu");
  /// Create a new builder for the module.
  Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

/**
 * Expr
 */

llvm::Value *IntegerLiteral::codegen() {
  return llvm::ConstantInt::get(*TheContext, llvm::APInt(64, value));
}

llvm::Value *FloatingLiteral::codegen() {
  return llvm::ConstantFP::get(*TheContext, llvm::APFloat(value));
}

llvm::Value *CharacterLiteral::codegen() { return nullptr; }

llvm::Value *StringLiteral::codegen() { return nullptr; }

llvm::Value *DeclRefExpr::codegen() { return LocalVariableTable[name].second; }

llvm::Value *ParenExpr::codegen() { return expr->codegen(); }

llvm::Value *UnaryOperator::codegen() {
  auto *r = right->codegen();
  if (!r) {
    return nullptr;
  }
  switch (op.type) {
  case NOT:
    return Builder->CreateNot(r);
  case SUB:
    if (getType() == "i64") {
      return Builder->CreateNeg(r);
    } else {
      return Builder->CreateFNeg(r);
    }
  default:
    break;
  }
  return nullptr;
}

llvm::Value *BinaryOperator::codegen() {
  auto *l = left->codegen();
  auto *r = right->codegen();
  if (!l || !r) {
    return nullptr;
  }
  /// assignment
  if (op.type == EQUAL) {
    debug("binary operator =");
    return Builder->CreateStore(l, r);
  }
  if (op.type == AND_OP) {
    return Builder->CreateAnd(l, r);
  }
  if (op.type == OR_OP) {
    return Builder->CreateOr(l, r);
  }

  if (getType() == "i64") {
    switch (op.type) {
    case ADD:
      return Builder->CreateAdd(l, r);
    case SUB:
      return Builder->CreateSub(l, r);
    case MUL:
      return Builder->CreateMul(l, r);
    case DIV:
      return Builder->CreateSDiv(l, r);
    case MOD:
      return Builder->CreateSRem(l, r);
    default:
      break;
    }
  } else {
    switch (op.type) {
    case ADD:
      return Builder->CreateFAdd(l, r);
    case SUB:
      return Builder->CreateFSub(l, r);
    case MUL:
      return Builder->CreateFMul(l, r);
    case DIV:
      return Builder->CreateFDiv(l, r);
    case MOD:
      return Builder->CreateFRem(l, r);
    default:
      break;
    }
  }
  return nullptr;
}

/**
 * Stmt
 */

void CompoundStmt::codegen() {
  for (auto &stmt : stmts) {
    stmt->codegen();
  }
}

void ExprStmt::codegen() { expr->codegen(); }

void DeclStmt::codegen() {
  if (dynamic_cast<VarDecl *>(decl.get())) {
    dynamic_cast<VarDecl *>(decl.get())->codegen();
  }
}

void ReturnStmt::codegen() {
  if (expr != nullptr) {
    auto retVal = expr->codegen();
    Builder->CreateRet(retVal);
  } else {
    Builder->CreateRetVoid();
  }
}

/**
 * Decl
 */

llvm::Value *VarDecl::codegen() {
  llvm::Type *varType = (type == "i64" ? llvm::Type::getInt64Ty(*TheContext)
                                       : llvm::Type::getDoubleTy(*TheContext));
  llvm::Constant *initializer =
      (init != nullptr ? (llvm::Constant *)init->codegen() : nullptr);

  if (scope == GLOBAL) {
    llvm::GlobalVariable *var = new llvm::GlobalVariable(
        *TheModule, varType, false, llvm::GlobalValue::ExternalLinkage,
        initializer, name);
    VariableTable[name] = std::make_pair(type, initializer);
    return var;
  } else {
    llvm::AllocaInst *var = Builder->CreateAlloca(varType, nullptr, name);
    if (init != nullptr) {
      /// TODO: store inst: ptr -> i64* or f64*
      Builder->CreateStore(initializer, var);
    }
    LocalVariableTable[name] = std::make_pair(type, initializer);
    return var;
  }
}

llvm::Value *ParamVarDecl::codegen() { return nullptr; }

llvm::Function *FunctionDecl::codegen() {
  llvm::Type *result;
  if (type.starts_with("void")) {
    result = llvm::Type::getVoidTy(*TheContext);
  } else if (type.starts_with("i64")) {
    result = llvm::Type::getInt64Ty(*TheContext);
  } else if (type.starts_with("f64")) {
    result = llvm::Type::getDoubleTy(*TheContext);
  }

  std::vector<llvm::Type *> params;

  llvm::FunctionType *funcType = llvm::FunctionType::get(result, params, false);
  llvm::Function *func = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, name, TheModule.get());

  llvm::BasicBlock *bb = llvm::BasicBlock::Create(*TheContext, "entry", func);
  Builder->SetInsertPoint(bb);

  body->codegen();
  return func;

  // func->eraseFromParent();
  // return nullptr;
}
/**
 * TranslationUnitDecl
 */

void TranslationUnitDecl::codegen() {
  for (auto &decl : decls) {
    if (dynamic_cast<VarDecl *>(decl.get())) {
      dynamic_cast<VarDecl *>(decl.get())->codegen();
    } else if (dynamic_cast<FunctionDecl *>(decl.get())) {
      dynamic_cast<FunctionDecl *>(decl.get())->codegen();
    } else {
      /// TODO: codegen error handling
    }
  }
}

} // namespace toyc
