// ! code generation implementation

#include <AST/AST.h>
#include <CodeGen/InterpreterCodeGen.h>
#include <Parser/InterpreterParser.h>
#include <Util.h>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace toyc {

void InterpreterIRVisitor::initialize() {
  context = std::make_unique<llvm::LLVMContext>();
  module = std::make_unique<llvm::Module>("toyc jit", *context);
  module->setDataLayout(JIT->getDataLayout());
  builder = std::make_unique<llvm::IRBuilder<>>(*context);

  FPM = std::make_unique<llvm::legacy::FunctionPassManager>(module.get());
  FPM->add(llvm::createInstructionCombiningPass());
  FPM->add(llvm::createReassociatePass());
  FPM->add(llvm::createGVNPass());
  FPM->add(llvm::createCFGSimplificationPass());
  FPM->doInitialization();
}

void InterpreterIRVisitor::resetReferGlobalVar() {
  for (auto &var : globalVarEnv) {
    if (auto &f = var.second) {
      f->refered = 0;
    }
  }
}

void InterpreterIRVisitor::resetReferFunctionProto() {
  for (auto &func : functionEnv) {
    if (auto &f = func.second) {
      f->refered = 0;
    }
  }
}

InterpreterIRVisitor::InterpreterIRVisitor() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  JIT = ExitOnErr(ToycJIT::create());
  initialize();
}

llvm::GlobalVariable *InterpreterIRVisitor::getGlobalVar(std::string name) {
  llvm::GlobalVariable *var = nullptr;
  if (auto &gvar = globalVarEnv[name]) {
    llvm::Type *varTy =
        (gvar->type == "i64" ? builder->getInt64Ty() : builder->getDoubleTy());

    if (gvar->refered == 0) {
      var = new llvm::GlobalVariable(*module, varTy, false,
                                     llvm::GlobalVariable::ExternalLinkage,
                                     nullptr, name);
      gvar->refered++;
    } else {
      var = module->getGlobalVariable(name);
    }
  }
  return var;
}

/// override getFunction
llvm::Function *InterpreterIRVisitor::getFunction(const FunctionDecl &decl) {
  std::string funcName = decl.getName();
  if (auto &fn = functionEnv[funcName]) {
    if (fn->refered != 0) {
      return module->getFunction(funcName);
    } else {
      fn->refered++;
    }
  }

  /// function return type
  llvm::Type *resultTy;
  if (decl.proto->type.starts_with("void")) {
    resultTy = llvm::Type::getVoidTy(*context);
  } else if (decl.proto->type.starts_with("i64")) {
    resultTy = llvm::Type::getInt64Ty(*context);
  } else if (decl.proto->type.starts_with("f64")) {
    resultTy = llvm::Type::getDoubleTy(*context);
  }

  /// function parameters
  std::vector<llvm::Type *> params;
  for (auto &param : decl.proto->params) {
    params.push_back(param->accept(*this));
  }

  /// create function
  llvm::FunctionType *funcTy = llvm::FunctionType::get(resultTy, params, false);
  llvm::Function *func = llvm::Function::Create(
      funcTy, llvm::Function::ExternalLinkage, decl.proto->name, *module);
  return func;
}

/**
 * Expr
 */

llvm::Value *InterpreterIRVisitor::codegen(const DeclRefExpr &expr) {
  std::string varName = expr.decl->getName();
  auto *id = varEnv[varName];
  if (id != nullptr) {
    auto *idVal = builder->CreateLoad(id->getAllocatedType(), id);
    if (idVal == nullptr) {
      throw CodeGenException(fstr("identifier '{}' not load", varName));
    }
    return idVal;
  }
  llvm::GlobalVariable *gid = getGlobalVar(varName);
  if (gid != nullptr) {
    auto *idVal = builder->CreateLoad(gid->getValueType(), gid);
    if (idVal == nullptr) {
      throw CodeGenException(fstr("identifier '{}' not load", varName));
    }
    return idVal;
  }
  throw CodeGenException(fstr("identifier '{}' not found", varName));
}

llvm::Value *InterpreterIRVisitor::codegen(const CallExpr &expr) {
  llvm::Function *callee =
      getFunction((const FunctionDecl &)*expr.callee->decl);
  if (callee == nullptr) {
    throw CodeGenException(
        fstr("function '{}' not declared", expr.callee->decl->getName()));
  }
  std::vector<llvm::Value *> argVals;
  for (auto &arg : expr.args) {
    llvm ::Value *argVal = arg->accept(*this);
    if (argVal == nullptr) {
      throw CodeGenException("params not exists");
    }
    argVals.push_back(argVal);
  }
  return builder->CreateCall(callee, argVals);
}

llvm::Value *InterpreterIRVisitor::codegen(const UnaryOperator &expr) {
  auto *e = expr.expr->accept(*this);
  if (e == nullptr) {
    throw CodeGenException("[UnaryOperator] the operand is null");
  }

  /// prepare for INC_OP and DEC_OP
  llvm::Value *oneVal;
  if (expr.type == "i64") {
    oneVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1);
  } else {
    oneVal = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), 1);
  }
  llvm::Value *var;
  if (auto *_left = dynamic_cast<DeclRefExpr *>(expr.expr.get())) {
    std::string varName = _left->decl->getName();
    var = varEnv[varName];
    if (var == nullptr) {
      var = getGlobalVar(varName);
    }
  }

  switch (expr.op.type) {
  case ADD:
    return e;
  case NOT:
    return builder->CreateNot(e);
  case SUB:
    if (expr.type == "i64") {
      return builder->CreateNeg(e);
    } else {
      return builder->CreateFNeg(e);
    }
  case INC_OP: {
    llvm::Value *updated;
    if (expr.type == "i64") {
      updated = builder->CreateNSWAdd(e, oneVal);
    } else {
      updated = builder->CreateFAdd(e, oneVal);
    }
    builder->CreateStore(updated, var);
    if (expr.side == POSTFIX) {
      return e;
    } else {
      return updated;
    }
  }
  case DEC_OP: {
    llvm::Value *updated;
    if (expr.type == "i64") {
      updated = builder->CreateNSWSub(e, oneVal);
    } else {
      updated = builder->CreateFSub(e, oneVal);
    }
    builder->CreateStore(updated, var);
    if (expr.side == POSTFIX) {
      return e;
    } else {
      return updated;
    }
  }
  default:
    throw CodeGenException(fstr(
        "[UnaryOperator] unimplemented unary operator '{}'", expr.op.value));
  }
}

llvm::Value *InterpreterIRVisitor::codegen(const BinaryOperator &expr) {
  llvm::Value *l, *r;

  if (expr.op.type == EQUAL) {
    if (auto *_left = dynamic_cast<DeclRefExpr *>(expr.left.get())) {
      std::string varName = _left->decl->getName();
      l = varEnv[varName];
      if (l == nullptr) {
        l = getGlobalVar(varName);
      }
    }
    r = expr.right->accept(*this);
    return builder->CreateStore(r, l);
  }

  l = expr.left->accept(*this);
  r = expr.right->accept(*this);
  if (l == nullptr || r == nullptr) {
    throw CodeGenException("[BinaryOperator] operands must be not null");
  }

  /// logical operation (no matter types)
  auto opTy = expr.op.type;
  if (opTy == AND_OP) {
    return builder->CreateAnd(l, r);
  }
  if (opTy == OR_OP) {
    return builder->CreateOr(l, r);
  }

  if (expr.type == "i64") {
    switch (opTy) {
    case ADD:
      return builder->CreateNSWAdd(l, r);
    case SUB:
      return builder->CreateNSWSub(l, r);
    case MUL:
      return builder->CreateNSWMul(l, r);
    case DIV:
      return builder->CreateSDiv(l, r);
    case MOD:
      return builder->CreateSRem(l, r);
    case EQ_OP:
      return builder->CreateICmpEQ(l, r);
    case NE_OP:
      return builder->CreateICmpNE(l, r);
    case LE_OP:
      return builder->CreateICmpSLE(l, r);
    case GE_OP:
      return builder->CreateICmpSGE(l, r);
    case LA:
      return builder->CreateICmpSLT(l, r);
    case RA:
      return builder->CreateICmpSGT(l, r);
    case LEFT_OP:
      return builder->CreateShl(l, r);
    case RIGHT_OP:
      return builder->CreateAShr(l, r);
    default:
      break;
    }
  } else if (expr.type == "f64") {
    switch (opTy) {
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
    case EQ_OP:
      return builder->CreateFCmpOEQ(l, r);
    case NE_OP:
      return builder->CreateFCmpONE(l, r);
    case LE_OP:
      return builder->CreateFCmpOLE(l, r);
    case GE_OP:
      return builder->CreateFCmpOGE(l, r);
    case LA:
      return builder->CreateFCmpOLT(l, r);
    case RA:
      return builder->CreateFCmpOGT(l, r);
    default:
      break;
    }
  }
  throw CodeGenException("[BinaryOperator] unimplemented binary operation");
}

/**
 * Decl
 */

llvm::Value *InterpreterIRVisitor::codegen(const VarDecl &decl) {
  llvm::Type *varTy =
      (decl.type == "i64" ? builder->getInt64Ty() : builder->getDoubleTy());
  llvm::Constant *initializer =
      (decl.init != nullptr ? (llvm::Constant *)decl.init->accept(*this)
                            : nullptr);

  if (decl.scope == GLOBAL) {
    llvm::GlobalVariable *var = new llvm::GlobalVariable(
        *module, varTy, false, llvm::GlobalVariable::ExternalLinkage,
        initializer, decl.name);
    globalVarEnv[decl.name] =
        std::make_unique<GlobalVar>(decl.name, decl.type, 0);
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

/**
 * Top
 */

void InterpreterIRVisitor::handleDeclaration(std::unique_ptr<Decl> &decl) {
  if (auto varDecl = dynamic_cast<VarDecl *>(decl.get())) {
    /// for variable declaration, all see as global variable
    varDecl->accept(*this);
    // dump();
    ExitOnErr(JIT->addModule(
        llvm::orc::ThreadSafeModule(std::move(module), std::move(context))));
    initialize();
    resetReferGlobalVar();
  } else if (auto funcDecl = dynamic_cast<FunctionDecl *>(decl.get())) {
    /// for function definition
    if (funcDecl->getKind() != DECLARATION) {
      funcDecl->accept(*this);
      functionEnv[funcDecl->getName()] = std::move(funcDecl->proto);
      if (funcDecl->getKind() == DEFINITION) {
        // dump();
        ExitOnErr(JIT->addModule(llvm::orc::ThreadSafeModule(
            std::move(module), std::move(context))));
        initialize();
        resetReferGlobalVar();
        resetReferFunctionProto();
      }
    }
  } else {
    throw CodeGenException("[TranslationUnitDecl] unsupported declaration");
  }
}

void InterpreterIRVisitor::handleStatement(std::unique_ptr<Stmt> &stmt) {
  auto proto = std::make_unique<FunctionProto>(
      "__anon_stmt__", "void", std::vector<std::unique_ptr<ParmVarDecl>>{}, 0);
  auto funcDecl =
      std::make_unique<FunctionDecl>(std::move(proto), std::move(stmt));
  if (funcDecl->accept(*this)) {
    // dump();
    auto resTracker = JIT->getMainJITDylib().createResourceTracker();
    auto tsm =
        llvm::orc::ThreadSafeModule(std::move(module), std::move(context));
    ExitOnErr(JIT->addModule(std::move(tsm), resTracker));
    initialize();
    resetReferGlobalVar();
    // resetReferFunctionProto();

    auto exprSym = ExitOnErr(JIT->lookup("__anon_stmt__"));
    if (funcDecl->getType() == "void") {
      void (*functionPtr)() = (void (*)())(intptr_t)exprSym.getAddress();
      functionPtr();
    } else {
      throw CodeGenException(
          fstr("not supported type '{}'", funcDecl->getType()));
    }
    ExitOnErr(resTracker->remove());
  } else {
    throw CodeGenException("generate anon function failed");
  }
}

void InterpreterIRVisitor::handleExpression(std::unique_ptr<Expr> &expr) {
  std::string type = expr->getType();
  auto stmt = std::make_unique<ReturnStmt>(std::move(expr));
  auto proto = std::make_unique<FunctionProto>(
      "__anon_expr__", std::move(type),
      std::vector<std::unique_ptr<ParmVarDecl>>{}, 0);
  auto funcDecl =
      std::make_unique<FunctionDecl>(std::move(proto), std::move(stmt));

  if (funcDecl->accept(*this)) {
    // dump();
    auto resTracker = JIT->getMainJITDylib().createResourceTracker();
    auto tsm =
        llvm::orc::ThreadSafeModule(std::move(module), std::move(context));
    ExitOnErr(JIT->addModule(std::move(tsm), resTracker));
    initialize();
    resetReferGlobalVar();
    resetReferFunctionProto();

    auto exprSym = ExitOnErr(JIT->lookup("__anon_expr__"));
    if (funcDecl->getType() == "i64") {
      int64_t (*functionPtr)() = (int64_t(*)())(intptr_t)exprSym.getAddress();
      std::cout << fstr("{}\n", functionPtr());
    } else if (funcDecl->getType() == "f64") {
      double (*functionPtr)() = (double (*)())(intptr_t)exprSym.getAddress();
      std::cout << fstr("{}\n", functionPtr());
    } else {
      throw CodeGenException(
          fstr("not supported type '{}'", funcDecl->getType()));
    }
    ExitOnErr(resTracker->remove());
  } else {
    throw CodeGenException("generate anon function failed");
  }
}

} // namespace toyc
