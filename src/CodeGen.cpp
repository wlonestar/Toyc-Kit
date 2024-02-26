// ! code generation implementation

#include <AST.h>
#include <CodeGen.h>
#include <Parser.h>
#include <Token.h>
#include <Util.h>

#include <llvm-16/llvm/IR/DerivedTypes.h>
#include <llvm-16/llvm/IR/GlobalValue.h>
#include <llvm-16/llvm/IR/Verifier.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <exception>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace toyc {

IRCodegenVisitor::IRCodegenVisitor() {
  /// Open a new context and module.
  context = std::make_unique<llvm::LLVMContext>();
  module = std::make_unique<llvm::Module>("toyc jit", *context);
  /// Set data layout and target triple explicitly
  module->setDataLayout(
      "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128");
  module->setTargetTriple("x86_64-pc-linux-gnu");
  /// Create a new builder for the module.
  builder = std::make_unique<llvm::IRBuilder<>>(*context);
}

void IRCodegenVisitor::modulePrint(llvm::raw_ostream &OS,
                                   llvm::AssemblyAnnotationWriter *AAW,
                                   bool ShouldPreserveUseListOrder,
                                   bool IsForDebug) const {
  module->print(OS, AAW, ShouldPreserveUseListOrder, IsForDebug);
}

bool IRCodegenVisitor::verifyModule(llvm::raw_ostream *OS,
                                    bool *BrokenDebugInfo) {
  return llvm::verifyModule(*module, OS, BrokenDebugInfo);
}

/**
 * Expr
 */

llvm::Value *IRCodegenVisitor::codegen(const IntegerLiteral &expr) {
  return llvm::ConstantInt::get((llvm::Type::getInt64Ty(*context)), expr.value);
}

llvm::Value *IRCodegenVisitor::codegen(const FloatingLiteral &expr) {
  return llvm::ConstantFP::get((llvm::Type::getFloatTy(*context)), expr.value);
}

llvm::Value *IRCodegenVisitor::codegen(const DeclRefExpr &expr) {
  /// TODO: need to fix bugs
  auto val = LocalVariableTable[expr.name];
  return std::get<2>(val);
}

llvm::Value *IRCodegenVisitor::codegen(const ParenExpr &expr) {
  return expr.expr->accept(*this);
}

llvm::Value *IRCodegenVisitor::codegen(const UnaryOperator &expr) {
  auto *r = expr.right->accept(*this);
  if (r == nullptr) {
    /// TODO: codegen error handling
    return nullptr;
  }
  switch (expr.op.type) {
  case NOT:
    return builder->CreateNot(r, "not");
  case SUB:
    if (expr.type == "i64") {
      return builder->CreateNeg(r, "neg");
    } else {
      return builder->CreateFNeg(r, "neg");
    }
  default:
    /// TODO: codegen error handling
    return nullptr;
  }
}

llvm::Value *IRCodegenVisitor::codegen(const BinaryOperator &expr) {
  llvm::Value *l = expr.left->accept(*this);
  llvm::Value *r = expr.right->accept(*this);
  if (auto *_left = dynamic_cast<DeclRefExpr *>(expr.left.get())) {
    l = std::get<2>(LocalVariableTable[_left->name]);
  }
  if (auto *_right = dynamic_cast<DeclRefExpr *>(expr.right.get())) {
    r = std::get<2>(LocalVariableTable[_right->name]);
  }

  /// TODO: assignment
  if (expr.op.type == EQUAL) {
    if (auto *_left = dynamic_cast<DeclRefExpr *>(expr.left.get())) {
      l = std::get<1>(LocalVariableTable[_left->name]);
    }
    return builder->CreateStore(r, l);
  }

  if (!l || !r) {
    /// TODO: codegen error handling
    return nullptr;
  }

  /// logical operation
  if (expr.op.type == AND_OP) {
    return builder->CreateAnd(l, r, "and");
  }
  if (expr.op.type == OR_OP) {
    return builder->CreateOr(l, r, "or");
  }

  if (expr.type == "i64") {
    switch (expr.op.type) {
    case ADD:
      return builder->CreateNSWAdd(l, r, "add");
    case SUB:
      return builder->CreateSub(l, r, "sub");
    case MUL:
      return builder->CreateMul(l, r, "mult");
    case DIV:
      return builder->CreateSDiv(l, r, "div");
    case MOD:
      return builder->CreateSRem(l, r, "rem");
    default:
      break;
    }
  } else {
    switch (expr.op.type) {
    case ADD:
      return builder->CreateFAdd(l, r, "add");
    case SUB:
      return builder->CreateFSub(l, r, "sub");
    case MUL:
      return builder->CreateFMul(l, r, "mult");
    case DIV:
      return builder->CreateFDiv(l, r, "div");
    case MOD:
      return builder->CreateFRem(l, r, "rem");
    default:
      break;
    }
  }
  /// TODO: codegen error handling
  return nullptr;
}

/**
 * Stmt
 */

llvm::Value *IRCodegenVisitor::codegen(const CompoundStmt &stmt) {
  /// TODO:
  for (auto &stmt : stmt.stmts) {
    stmt->accept(*this);
  }
  return nullptr;
}

llvm::Value *IRCodegenVisitor::codegen(const ExprStmt &stmt) {
  /// TODO:
  return stmt.expr->accept(*this);
}

llvm::Value *IRCodegenVisitor::codegen(const DeclStmt &stmt) {
  /// TODO:
  if (auto var = dynamic_cast<VarDecl *>(stmt.decl.get())) {
    /// TODO: decl
  }
  return nullptr;
}

llvm::Value *IRCodegenVisitor::codegen(const ReturnStmt &stmt) {
  /// TODO:
  if (stmt.expr != nullptr) {
    auto retVal = stmt.expr->accept(*this);
    return builder->CreateRet(retVal);
  } else {
    return builder->CreateRetVoid();
  }
  return nullptr;
}

/**
 * Decl
 */

llvm::Value *IRCodegenVisitor::codegen(const VarDecl &decl) {
  llvm::Type *varTy =
      (decl.type == "i64" ? builder->getInt64Ty() : builder->getDoubleTy());
  llvm::Constant *initializer =
      (decl.init != nullptr ? (llvm::Constant *)decl.init->accept(*this)
                            : nullptr);

  if (decl.scope == GLOBAL) {
    llvm::GlobalVariable *var = new llvm::GlobalVariable(
        *module, varTy, false, llvm::GlobalVariable::ExternalLinkage,
        initializer, decl.name);
    VariableTable[decl.name] = std::make_pair(decl.type, initializer);
    return var;
  } else {
    llvm::AllocaInst *var = builder->CreateAlloca(varTy, nullptr, decl.name);
    if (decl.init != nullptr) {
      /// TODO: store inst: ptr -> i64* or f64*
      builder->CreateStore(initializer, var);
    }
    LocalVariableTable[decl.name] =
        std::make_tuple(decl.type, var, initializer);
    return var;
  }
}

llvm::Value *IRCodegenVisitor::codegen(const ParamVarDecl &decl) {
  /// TODO:
  return nullptr;
}

llvm::Function *IRCodegenVisitor::codegen(const FunctionDecl &decl) {
  llvm::Type *resultTy;
  if (decl.type.starts_with("void")) {
    resultTy = llvm::Type::getVoidTy(*context);
  } else if (decl.type.starts_with("i64")) {
    resultTy = llvm::Type::getInt64Ty(*context);
  } else if (decl.type.starts_with("f64")) {
    resultTy = llvm::Type::getDoubleTy(*context);
  }

  std::vector<llvm::Type *> params;

  llvm::FunctionType *funcTy = llvm::FunctionType::get(resultTy, params, false);
  llvm::Function *func = llvm::Function::Create(
      funcTy, llvm::Function::ExternalLinkage, decl.name, *module);

  llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "entry", func);
  builder->SetInsertPoint(bb);

  if (decl.body != nullptr) {
    decl.body->accept(*this);
  }
  return func;
}

/**
 * TranslationUnitDecl
 */

void IRCodegenVisitor::codegen(const TranslationUnitDecl &decl) {
  /// TODO:
  for (auto &d : decl.decls) {
    /// TODO:
    if (dynamic_cast<VarDecl *>(d.get())) {
      dynamic_cast<VarDecl *>(d.get())->accept(*this);
    } else if (dynamic_cast<FunctionDecl *>(d.get())) {
      /// TODO: change to read from FunctionTable
      /// (avoid seperating function declaration and definition)
      dynamic_cast<FunctionDecl *>(d.get())->accept(*this);
    } else {
      /// TODO: codegen error handling
    }
  }
}

} // namespace toyc
