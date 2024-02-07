// ! code generation implementation

#include <AST.h>
#include <CodeGen.h>
#include <Parser.h>
#include <Token.h>

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
  /// TODO: support more int type
  return llvm::ConstantInt::get(*TheContext,
                                llvm::APInt(64, std::get<0>(value)));
}

llvm::Value *FloatingLiteral::codegen() {
  /// TODO: support float, double and long double
  return llvm::ConstantFP::get(*TheContext, llvm::APFloat(std::get<0>(value)));
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

llvm::Value *UnaryOperator::codegen() { return nullptr; }

llvm::Value *BinaryOperator::codegen() {
  auto *l = left->codegen();
  auto *r = right->codegen();
  if (!l || !r) {
    return nullptr;
  }
  switch (op.type) {
  case ADD:
    return Builder->CreateAdd(l, r);
  default:
    break;
  }
  return nullptr;
}

/**
 * Decl
 */

llvm::Value *VarDecl::codegen() {
  llvm::Type *intType = llvm::Type::getInt32Ty(*TheContext);
  auto initializer = (llvm::Constant *)init->codegen();

  llvm::GlobalVariable *globalVar = new llvm::GlobalVariable(
      *TheModule, intType, false, llvm::GlobalValue::ExternalLinkage,
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
  return nullptr;
}

} // namespace toyc
