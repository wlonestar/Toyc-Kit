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

void InterpreterIRVisitor::Initialize() {
  context_ = std::make_unique<llvm::LLVMContext>();
  module_ = std::make_unique<llvm::Module>("toyc jit", *context_);
  module_->setDataLayout(jit_->GetDataLayout());
  builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);

  fpm_ = std::make_unique<llvm::legacy::FunctionPassManager>(module_.get());
  fpm_->add(llvm::createInstructionCombiningPass());
  fpm_->add(llvm::createReassociatePass());
  fpm_->add(llvm::createGVNPass());
  fpm_->add(llvm::createCFGSimplificationPass());
  fpm_->doInitialization();
}

void InterpreterIRVisitor::ResetReferGlobalVar() {
  for (auto &var : global_var_env_) {
    if (auto &f = var.second) {
      f->refered_ = 0;
    }
  }
}

void InterpreterIRVisitor::ResetReferFunctionProto() {
  for (auto &func : function_env_) {
    if (auto &f = func.second) {
      f->refered_ = 0;
    }
  }
}

InterpreterIRVisitor::InterpreterIRVisitor() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  jit_ = exit_on_err_(ToycJIT::Create());
  Initialize();
}

auto InterpreterIRVisitor::GetGlobalVar(const std::string &name)
    -> llvm::GlobalVariable * {
  llvm::GlobalVariable *var = nullptr;
  if (auto &gvar = global_var_env_[name]) {
    llvm::Type *var_ty = (gvar->type_ == "i64" ? builder_->getInt64Ty()
                                               : builder_->getDoubleTy());

    if (gvar->refered_ == 0) {
      var = new llvm::GlobalVariable(*module_, var_ty, false,
                                     llvm::GlobalVariable::ExternalLinkage,
                                     nullptr, name);
      gvar->refered_++;
    } else {
      var = module_->getGlobalVariable(name);
    }
  }
  return var;
}

/// override getFunction
auto InterpreterIRVisitor::GetFunction(const FunctionDecl &decl)
    -> llvm::Function * {
  std::string func_name = decl.GetName();
  if (auto &fn = function_env_[func_name]) {
    if (fn->refered_ != 0) {
      return module_->getFunction(func_name);
    }
    fn->refered_++;
  }

  /// function return type
  llvm::Type *result_ty;
  if (decl.proto_->type_.starts_with("void")) {
    result_ty = llvm::Type::getVoidTy(*context_);
  } else if (decl.proto_->type_.starts_with("i64")) {
    result_ty = llvm::Type::getInt64Ty(*context_);
  } else if (decl.proto_->type_.starts_with("f64")) {
    result_ty = llvm::Type::getDoubleTy(*context_);
  }

  /// function parameters
  std::vector<llvm::Type *> params;
  for (auto &param : decl.proto_->params_) {
    params.push_back(param->Accept(*this));
  }

  /// create function
  llvm::FunctionType *func_ty =
      llvm::FunctionType::get(result_ty, params, false);
  llvm::Function *func = llvm::Function::Create(
      func_ty, llvm::Function::ExternalLinkage, decl.proto_->name_, *module_);
  return func;
}

/**
 * Expr
 */

auto InterpreterIRVisitor::Codegen(const DeclRefExpr &expr) -> llvm::Value * {
  std::string var_name = expr.decl_->GetName();
  llvm::AllocaInst *id = var_env_[var_name];
  if (id != nullptr) {
    llvm::LoadInst *id_val = builder_->CreateLoad(id->getAllocatedType(), id);
    if (id_val == nullptr) {
      throw CodeGenException(
          makeString("local identifier '{}' not load", var_name));
    }
    return id_val;
  }
  llvm::GlobalVariable *gid = GetGlobalVar(var_name);
  if (gid != nullptr) {
    llvm::LoadInst *id_val = builder_->CreateLoad(gid->getValueType(), gid);
    if (id_val == nullptr) {
      throw CodeGenException(
          makeString("global identifier '{}' not load", var_name));
    }
    return id_val;
  }
  throw CodeGenException(makeString("identifier '{}' not found", var_name));
}

auto InterpreterIRVisitor::Codegen(const CallExpr &expr) -> llvm::Value * {
  llvm::Function *callee =
      // GetFunction((const FunctionDecl &)*expr.callee_->decl_);
      GetFunction(reinterpret_cast<const FunctionDecl &>(*expr.callee_->decl_));
  if (callee == nullptr) {
    throw CodeGenException(makeString("function '{}' not declared",
                                      expr.callee_->decl_->GetName()));
  }
  std::vector<llvm::Value *> arg_vals;
  for (auto &arg : expr.args_) {
    llvm ::Value *arg_val = arg->Accept(*this);
    if (arg_val == nullptr) {
      throw CodeGenException("params not exists");
    }
    arg_vals.push_back(arg_val);
  }
  return builder_->CreateCall(callee, arg_vals);
}

auto InterpreterIRVisitor::Codegen(const UnaryOperator &expr) -> llvm::Value * {
  llvm::Value *e = expr.expr_->Accept(*this);
  if (e == nullptr) {
    throw CodeGenException("[UnaryOperator] the operand is null");
  }

  /// prepare for INC_OP and DEC_OP
  llvm::Value *one_val;
  if (expr.type_ == "i64") {
    one_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 1);
  } else {
    one_val = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context_), 1);
  }
  llvm::Value *ptr;
  if (auto *left = dynamic_cast<DeclRefExpr *>(expr.expr_.get())) {
    std::string var_name = left->decl_->GetName();
    ptr = var_env_[var_name];
    if (ptr == nullptr) {
      ptr = GetGlobalVar(var_name);
    }
  }

  switch (expr.op_.type_) {
  case ADD:
    return e;
  case NOT:
    return builder_->CreateNot(e);
  case SUB:
    if (expr.type_ == "i64") {
      return builder_->CreateNeg(e);
    } else {
      return builder_->CreateFNeg(e);
    }
  case INC_OP: {
    llvm::Value *updated;
    if (expr.type_ == "i64") {
      updated = builder_->CreateNSWAdd(e, one_val);
    } else {
      updated = builder_->CreateFAdd(e, one_val);
    }
    builder_->CreateStore(updated, ptr);
    if (expr.side_ == POSTFIX) {
      return e;
    }
    return updated;
  }
  case DEC_OP: {
    llvm::Value *updated;
    if (expr.type_ == "i64") {
      updated = builder_->CreateNSWSub(e, one_val);
    } else {
      updated = builder_->CreateFSub(e, one_val);
    }
    builder_->CreateStore(updated, ptr);
    if (expr.side_ == POSTFIX) {
      return e;
    }
    return updated;
  }
  default:
    throw CodeGenException(makeString(
        "[UnaryOperator] unimplemented unary operator '{}'", expr.op_.value_));
  }
}

auto InterpreterIRVisitor::Codegen(const BinaryOperator &expr)
    -> llvm::Value * {
  llvm::Value *l;
  llvm::Value *r;

  if (expr.op_.type_ == EQUAL) {
    if (auto *left = dynamic_cast<DeclRefExpr *>(expr.left_.get())) {
      std::string var_name = left->decl_->GetName();
      l = var_env_[var_name];
      if (l == nullptr) {
        l = GetGlobalVar(var_name);
      }
    }
    r = expr.right_->Accept(*this);
    return builder_->CreateStore(r, l);
  }

  l = expr.left_->Accept(*this);
  r = expr.right_->Accept(*this);
  if (l == nullptr || r == nullptr) {
    throw CodeGenException("[BinaryOperator] operands must be not null");
  }

  /// logical operation (no matter types)
  TokenTy op_ty = expr.op_.type_;
  if (op_ty == AND_OP) {
    return builder_->CreateAnd(l, r);
  }
  if (op_ty == OR_OP) {
    return builder_->CreateOr(l, r);
  }

  if (expr.type_ == "i64") {
    switch (op_ty) {
    case ADD:
      return builder_->CreateNSWAdd(l, r);
    case SUB:
      return builder_->CreateNSWSub(l, r);
    case MUL:
      return builder_->CreateNSWMul(l, r);
    case DIV:
      return builder_->CreateSDiv(l, r);
    case MOD:
      return builder_->CreateSRem(l, r);
    case EQ_OP:
      return builder_->CreateICmpEQ(l, r);
    case NE_OP:
      return builder_->CreateICmpNE(l, r);
    case LE_OP:
      return builder_->CreateICmpSLE(l, r);
    case GE_OP:
      return builder_->CreateICmpSGE(l, r);
    case LT:
      return builder_->CreateICmpSLT(l, r);
    case RT:
      return builder_->CreateICmpSGT(l, r);
    case LEFT_OP:
      return builder_->CreateShl(l, r);
    case RIGHT_OP:
      return builder_->CreateAShr(l, r);
    default:
      break;
    }
  } else if (expr.type_ == "f64") {
    switch (op_ty) {
    case ADD:
      return builder_->CreateFAdd(l, r);
    case SUB:
      return builder_->CreateFSub(l, r);
    case MUL:
      return builder_->CreateFMul(l, r);
    case DIV:
      return builder_->CreateFDiv(l, r);
    case MOD:
      return builder_->CreateFRem(l, r);
    case EQ_OP:
      return builder_->CreateFCmpOEQ(l, r);
    case NE_OP:
      return builder_->CreateFCmpONE(l, r);
    case LE_OP:
      return builder_->CreateFCmpOLE(l, r);
    case GE_OP:
      return builder_->CreateFCmpOGE(l, r);
    case LT:
      return builder_->CreateFCmpOLT(l, r);
    case RT:
      return builder_->CreateFCmpOGT(l, r);
    default:
      break;
    }
  }
  throw CodeGenException("[BinaryOperator] unimplemented binary operation");
}

/**
 * Decl
 */

auto InterpreterIRVisitor::Codegen(const VarDecl &decl) -> llvm::Value * {
  llvm::Type *var_ty =
      (decl.type_ == "i64" ? builder_->getInt64Ty() : builder_->getDoubleTy());
  llvm::Constant *initializer =
      // (decl.init_ != nullptr ? (llvm::Constant *)decl.init_->Accept(*this)
      //                        : nullptr);
      (decl.init_ != nullptr
           ? reinterpret_cast<llvm::Constant *>(decl.init_->Accept(*this))
           : nullptr);

  if (decl.scope_ == GLOBAL) {
    auto *var = new llvm::GlobalVariable(*module_, var_ty, false,
                                         llvm::GlobalVariable::ExternalLinkage,
                                         initializer, decl.name_);
    global_var_env_[decl.name_] =
        std::make_unique<GlobalVar>(decl.name_, decl.type_, 0);
    return var;
  }

  llvm::AllocaInst *ptr = builder_->CreateAlloca(var_ty, nullptr);
  if (decl.init_ != nullptr) {
    builder_->CreateStore(initializer, ptr);
  }
  var_env_[decl.name_] = ptr;
  return ptr;
}

/**
 * Top
 */

void InterpreterIRVisitor::HandleDeclaration(std::unique_ptr<Decl> &decl) {
  if (auto *var_decl = dynamic_cast<VarDecl *>(decl.get())) {
    /// for variable declaration, all see as global variable
    llvm::Type *var_ty =
        (var_decl->GetType() == "i64" ? builder_->getInt64Ty()
                                      : builder_->getDoubleTy());
    llvm::Value *zero_val;
    if (var_decl->GetType() == "i64") {
      zero_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0);
    } else {
      zero_val = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context_), 0);
    }
    llvm::Constant *initializer =
        // (var_decl->init_ != nullptr && var_decl->init_->IsConstant()
        //      ? (llvm::Constant *)var_decl->init_->Accept(*this)
        //      : (llvm::Constant *)zero_val);
        (var_decl->init_ != nullptr && var_decl->init_->IsConstant()
             ? reinterpret_cast<llvm::Constant *>(
                   var_decl->init_->Accept(*this))
             : reinterpret_cast<llvm::Constant *>(zero_val));

    /// variable definition
    auto *var = new llvm::GlobalVariable(*module_, var_ty, false,
                                         llvm::GlobalVariable::ExternalLinkage,
                                         initializer, var_decl->GetName());
    global_var_env_[var_decl->GetName()] = std::make_unique<GlobalVar>(
        var_decl->GetName(), var_decl->GetType(), 0);

    exit_on_err_(jit_->AddModule(
        llvm::orc::ThreadSafeModule(std::move(module_), std::move(context_))));
    Initialize();
    ResetReferGlobalVar();

    /// assign in function
    if (var_decl->init_ != nullptr && !var_decl->init_->IsConstant()) {
      auto decl_ref = std::make_unique<DeclRefExpr>(
          std::make_unique<VarDecl>(var_decl->GetName(), var_decl->GetType()));
      auto assign_expr = std::make_unique<BinaryOperator>(
          Token(EQUAL, "="), std::move(decl_ref), std::move(var_decl->init_),
          var_decl->GetType());
      auto stmt = std::make_unique<ExprStmt>(std::move(assign_expr));
      auto proto = std::make_unique<FunctionProto>(
          "__wrapped__var_init__", "void",
          std::vector<std::unique_ptr<ParmVarDecl>>{}, 0);
      auto func_decl =
          std::make_unique<FunctionDecl>(std::move(proto), std::move(stmt));
      func_decl->Accept(*this);

      auto res_tracker = jit_->GetMainJITDylib().createResourceTracker();
      auto tsm =
          llvm::orc::ThreadSafeModule(std::move(module_), std::move(context_));
      exit_on_err_(jit_->AddModule(std::move(tsm), res_tracker));
      Initialize();
      ResetReferGlobalVar();

      auto expr_sym = exit_on_err_(jit_->Lookup("__wrapped__var_init__"));
      if (func_decl->GetType() == "void") {
        auto function_ptr =
            (void (*)()) static_cast<intptr_t>(expr_sym.getAddress());
        function_ptr();
      } else {
        throw CodeGenException(
            makeString("not supported type '{}'", func_decl->GetType()));
      }
      exit_on_err_(res_tracker->remove());
    }
  } else if (auto *func_decl = dynamic_cast<FunctionDecl *>(decl.get())) {
    /// for function definition
    if (func_decl->GetKind() != DECLARATION) {
      func_decl->Accept(*this);
      function_env_[func_decl->GetName()] = std::move(func_decl->proto_);
      if (func_decl->GetKind() == DEFINITION) {
        exit_on_err_(jit_->AddModule(llvm::orc::ThreadSafeModule(
            std::move(module_), std::move(context_))));
        Initialize();
        ResetReferGlobalVar();
        ResetReferFunctionProto();
      }
    }
  } else {
    throw CodeGenException("[TranslationUnitDecl] unsupported declaration");
  }
}

void InterpreterIRVisitor::HandleStatement(std::unique_ptr<Stmt> &stmt) {
  auto proto = std::make_unique<FunctionProto>(
      "__wrapped__stmt__", "void", std::vector<std::unique_ptr<ParmVarDecl>>{},
      0);
  auto func_decl =
      std::make_unique<FunctionDecl>(std::move(proto), std::move(stmt));
  if (func_decl->Accept(*this) != nullptr) {
    auto res_tracker = jit_->GetMainJITDylib().createResourceTracker();
    auto tsm =
        llvm::orc::ThreadSafeModule(std::move(module_), std::move(context_));
    exit_on_err_(jit_->AddModule(std::move(tsm), res_tracker));
    Initialize();
    ResetReferGlobalVar();
    // resetReferFunctionProto();

    auto expr_sym = exit_on_err_(jit_->Lookup("__wrapped__stmt__"));
    if (func_decl->GetType() == "void") {
      auto function_ptr =
          (void (*)()) static_cast<intptr_t>(expr_sym.getAddress());
      function_ptr();
    } else {
      throw CodeGenException(
          makeString("not supported type '{}'", func_decl->GetType()));
    }
    exit_on_err_(res_tracker->remove());
  } else {
    throw CodeGenException("generate anon function failed");
  }
}

void InterpreterIRVisitor::HandleExpression(std::unique_ptr<Expr> &expr) {
  std::string type = expr->GetType();
  auto stmt = std::make_unique<ReturnStmt>(std::move(expr));
  auto proto = std::make_unique<FunctionProto>(
      "__wrapped__expr__", std::move(type),
      std::vector<std::unique_ptr<ParmVarDecl>>{}, 0);
  auto func_decl =
      std::make_unique<FunctionDecl>(std::move(proto), std::move(stmt));

  if (func_decl->Accept(*this) != nullptr) {
    auto res_tracker = jit_->GetMainJITDylib().createResourceTracker();
    auto tsm =
        llvm::orc::ThreadSafeModule(std::move(module_), std::move(context_));
    exit_on_err_(jit_->AddModule(std::move(tsm), res_tracker));
    Initialize();
    ResetReferGlobalVar();
    ResetReferFunctionProto();

    auto expr_sym = exit_on_err_(jit_->Lookup("__wrapped__expr__"));
    if (func_decl->GetType() == "i64") {
      auto function_ptr =
          (int64_t(*)()) static_cast<intptr_t>(expr_sym.getAddress());
      std::cout << makeString("{}\n", function_ptr());
    } else if (func_decl->GetType() == "f64") {
      auto function_ptr =
          (double (*)()) static_cast<intptr_t>(expr_sym.getAddress());
      std::cout << makeString("{}\n", function_ptr());
    } else {
      throw CodeGenException(
          makeString("not supported type '{}'", func_decl->GetType()));
    }
    exit_on_err_(res_tracker->remove());
  } else {
    throw CodeGenException("generate anon function failed");
  }
}

} // namespace toyc
