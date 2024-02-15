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
#include <utility>

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
  // if (auto search = VariableTable.find(name); search != VariableTable.end())
  // {
  //   return search->second.second;
  // } else {
  //   return nullptr;
  // }
  return VariableTable[name].second;
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
 * ExprStmt
 */

llvm::Value *ExprStmt::codegen() { return expr->codegen(); }

/**
 * Decl
 */

llvm::Value *VarDecl::codegen() {
  llvm::Type *varType = (type == "i64" ? llvm::Type::getInt64Ty(*TheContext)
                                       : llvm::Type::getDoubleTy(*TheContext));

  llvm::Constant *initializer =
      (init != nullptr ? (llvm::Constant *)init->codegen() : nullptr);

  llvm::GlobalVariable *var = new llvm::GlobalVariable(
      *TheModule, varType, false, llvm::GlobalValue::ExternalLinkage,
      initializer, name);

  VariableTable[name] = std::make_pair(type, initializer);
  return var;
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
