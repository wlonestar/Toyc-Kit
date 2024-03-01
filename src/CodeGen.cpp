// ! code generation implementation

#include <AST.h>
#include <CodeGen.h>
#include <Parser.h>
#include <Token.h>
#include <Util.h>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/Triple.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>

#include <exception>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace toyc {

IRCodegenVisitor::IRCodegenVisitor() {
  /// Open a new context and module.
  context = std::make_unique<llvm::LLVMContext>();
  /// Create a new builder for the module.
  builder = std::unique_ptr<llvm::IRBuilder<>>(new llvm::IRBuilder<>(*context));
  module = std::make_unique<llvm::Module>("toyc jit", *context);

  module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
}

void IRCodegenVisitor::dumpIR(llvm::raw_ostream &os) {
  module->print(os, nullptr, false, false);
}

void IRCodegenVisitor::verifyModule(llvm::raw_ostream &os) {
  std::string info;
  llvm::raw_string_ostream ros(info);
  llvm::verifyModule(*module, &ros, nullptr);
  os << "\033[1;31mverify:\n"
     << (info.size() == 0 ? "(null)" : info) << "\033[0m\n";
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
  auto *id = varEnv[expr.decl->name];
  if (id != nullptr) {
    auto *idVal = builder->CreateLoad(id->getAllocatedType(), id);
    if (idVal == nullptr) {
      throw CodeGenException(fstr("identifier '{}' not load", expr.decl->name));
    }
    return idVal;
  }
  auto *gid = globalVarEnv[expr.decl->name];
  if (gid != nullptr) {
    auto *idVal = builder->CreateLoad(gid->getValueType(), gid);
    if (idVal == nullptr) {
      throw CodeGenException(fstr("identifier '{}' not load", expr.decl->name));
    }
    return idVal;
  }
  throw CodeGenException(fstr("identifier '{}' not found", expr.decl->name));
}

llvm::Value *IRCodegenVisitor::codegen(const ParenExpr &expr) {
  return expr.expr->accept(*this);
}

llvm::Value *IRCodegenVisitor::codegen(const UnaryOperator &expr) {
  auto *r = expr.right->accept(*this);
  if (r == nullptr) {
    throw CodeGenException("[UnaryOperator] the operand is null");
  }
  switch (expr.op.type) {
  case NOT:
    return builder->CreateNot(r);
  case SUB:
    if (expr.type == "i64") {
      return builder->CreateNeg(r);
    } else {
      return builder->CreateFNeg(r);
    }
  default:
    throw CodeGenException(fstr(
        "[UnaryOperator] unimplemented unary operator '{}'", expr.op.value));
  }
}

llvm::Value *IRCodegenVisitor::codegen(const BinaryOperator &expr) {
  llvm::Value *l, *r;

  if (expr.op.type == EQUAL) {
    if (auto *_left = dynamic_cast<DeclRefExpr *>(expr.left.get())) {
      l = varEnv[_left->decl->name];
    }
    r = expr.right->accept(*this);
    return builder->CreateStore(r, l);
  }

  l = expr.left->accept(*this);
  r = expr.right->accept(*this);
  if (l == nullptr || r == nullptr) {
    throw CodeGenException("[BinaryOperator] operands must be not null");
  }

  /// logical operation
  if (expr.op.type == AND_OP) {
    return builder->CreateAnd(l, r);
  }
  if (expr.op.type == OR_OP) {
    return builder->CreateOr(l, r);
  }

  if (expr.type == "i64") {
    switch (expr.op.type) {
    case ADD:
      return builder->CreateAdd(l, r);
    case SUB:
      return builder->CreateSub(l, r);
    case MUL:
      return builder->CreateMul(l, r);
    case DIV:
      return builder->CreateSDiv(l, r);
    case MOD:
      return builder->CreateSRem(l, r);
    default:
      break;
    }
  } else if (expr.type == "f64") {
    switch (expr.op.type) {
    case ADD:
      return builder->CreateFAdd(l, r);
    case SUB:
      return builder->CreateFSub(l, r);
    case MUL:
      return builder->CreateFMul(l, r);
    case DIV:
      return builder->CreateFDiv(l, r);
    case MOD:
      return builder->CreateFRem(l, r);
    default:
      break;
    }
  }
  throw CodeGenException("[BinaryOperator] unimplemented binary operation");
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
  if (auto var = dynamic_cast<VarDecl *>(stmt.decl.get())) {
    return var->accept(*this);
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
    globalVarEnv[decl.name] = var;
    printGlobalVarEnv();
    return var;
  } else {
    llvm::AllocaInst *var = builder->CreateAlloca(varTy, nullptr);
    if (decl.init != nullptr) {
      builder->CreateStore(initializer, var);
    }
    varEnv[decl.name] = var;
    return var;
  }
}

llvm::Value *IRCodegenVisitor::codegen(const ParamVarDecl &decl) {
  /// TODO:
  return nullptr;
}

llvm::Function *IRCodegenVisitor::codegen(const FunctionDecl &decl) {
  clearVarEnv(); /// clear local variable table

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

  llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "", func);
  builder->SetInsertPoint(bb);

  if (decl.body != nullptr) {
    decl.body->accept(*this);
    if (resultTy == llvm::Type::getVoidTy(*context)) {
      builder->CreateRetVoid();
    }
  }
  printVarEnv();
  return func;
}

/**
 * TranslationUnitDecl
 */

void IRCodegenVisitor::codegen(const TranslationUnitDecl &decl) {
  for (auto &d : decl.decls) {
    if (dynamic_cast<VarDecl *>(d.get())) {
      dynamic_cast<VarDecl *>(d.get())->accept(*this);
    } else if (dynamic_cast<FunctionDecl *>(d.get())) {
      /// TODO: change to read from FunctionTable
      /// (avoid seperating function declaration and definition)
      dynamic_cast<FunctionDecl *>(d.get())->accept(*this);
    } else {
      throw CodeGenException("[TranslationUnitDecl] unsupported declaration");
    }
  }
}

} // namespace toyc
