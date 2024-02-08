// ! code generation implementation

#include <AST.h>
#include <CodeGen.h>
#include <Parser.h>
#include <Token.h>
#include <Util.h>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <memory>

namespace toyc {

std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;

void initializeModule() {
  /// Open a new context and module.
  TheContext = std::make_unique<llvm::LLVMContext>();
  TheModule = std::make_unique<llvm::Module>("toyc jit", *TheContext);
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

llvm::Value *DeclRefExpr::codegen() {
  llvm::Value *value = VariableTable[name];
  if (!value) {
    /// TODO: code generation error handling
    return nullptr;
  }
  return value;
}

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
    case AND_OP:
      return Builder->CreateAnd(l, r);
    case OR_OP:
      return Builder->CreateOr(l, r);
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
    case AND_OP:
      return Builder->CreateAnd(l, r);
    case OR_OP:
      return Builder->CreateOr(l, r);
    default:
      break;
    }
  }
  return nullptr;
}

/**
 * Decl
 */

llvm::Value *VarDecl::codegen() {
  llvm::Type *varType;
  if (type == "i64") {
    varType = llvm::Type::getInt64Ty(*TheContext);
  } else {
    varType = llvm::Type::getDoubleTy(*TheContext);
  }

  llvm::Constant *initializer = nullptr;
  if (init != nullptr) {
    initializer = (llvm::Constant *)init->codegen();
  }

  llvm::GlobalVariable *globalVar = new llvm::GlobalVariable(
      *TheModule, varType, false, llvm::GlobalValue::ExternalLinkage,
      initializer, name);

  VariableTable.insert({name, initializer});
  return globalVar;
}

/**
 * TranslationUnitDecl
 */

llvm::Value *TranslationUnitDecl::codegen() {
  for (auto &decl : decls) {
    decl->codegen();
  }
  /// TODO: for a beter return type
  return nullptr;
}

} // namespace toyc
