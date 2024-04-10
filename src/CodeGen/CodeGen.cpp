// ! code generation implementation

#include <AST/AST.h>
#include <CodeGen/CodeGen.h>
#include <Lexer/Token.h>
#include <Parser/Parser.h>
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

#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace toyc {

/**
 * Base IR visitor
 */

void BaseIRVisitor::PrintVarEnv() {
  std::cout << "\033[1;32mVarEnv:\n";
  for (auto &[first, second] : var_env_) {
    std::string value_str;
    llvm::raw_string_ostream ros(value_str);
    if (second != nullptr) {
      second->print(ros);
    } else {
      ros << "<null>";
    }
    std::cout << makeString("  <var> '{}': {}\n", first, value_str);
  }
  std::cout << "\033[0m\n";
}

void BaseIRVisitor::Dump(llvm::raw_ostream &os) {
  module_->print(os, nullptr, false, false);
}

auto BaseIRVisitor::VerifyModule(llvm::raw_ostream &os) -> bool {
  std::string info;
  llvm::raw_string_ostream ros(info);
  llvm::verifyModule(*module_, &ros, nullptr);
  if (info.empty()) {
    return true;
  }
  os << "\033[1;31mverify:\n" << info << "\033[0m\n";
  return false;
}

auto BaseIRVisitor::GetFunction(const FunctionDecl &decl) -> llvm::Function * {
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

auto BaseIRVisitor::Codegen(const IntegerLiteral &expr) -> llvm::Value * {
  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), expr.value_);
}

auto BaseIRVisitor::Codegen(const FloatingLiteral &expr) -> llvm::Value * {
  return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context_), expr.value_);
}

auto BaseIRVisitor::Codegen(const ImplicitCastExpr &expr) -> llvm::Value * {
  if (expr.type_ == expr.expr_->GetType()) {
    return expr.expr_->Accept(*this);
  }
  llvm::Value *value = expr.expr_->Accept(*this);
  if (expr.type_ == "f64" && expr.expr_->GetType() == "i64") {
    return builder_->CreateSIToFP(value, llvm::Type::getDoubleTy(*context_));
  }
  if (expr.type_ == "i64" && expr.expr_->GetType() == "f64") {
    return builder_->CreateFPToSI(value, llvm::Type::getInt64Ty(*context_));
  }
  throw CodeGenException("not implemented!");
}

auto BaseIRVisitor::Codegen(const ParenExpr &expr) -> llvm::Value * {
  return expr.expr_->Accept(*this);
}

/**
 * Stmt
 */

auto BaseIRVisitor::Codegen(const CompoundStmt &stmt) -> llvm::Value * {
  llvm::Value *ret_val = nullptr;
  for (auto &stmt : stmt.stmts_) {
    if (llvm::Value *ret = stmt->Accept(*this)) {
      ret_val = ret;
    }
  }
  return ret_val;
}

auto BaseIRVisitor::Codegen(const ExprStmt &stmt) -> llvm::Value * {
  return stmt.expr_->Accept(*this);
}

auto BaseIRVisitor::Codegen(const DeclStmt &stmt) -> llvm::Value * {
  if (auto *var = dynamic_cast<VarDecl *>(stmt.decl_.get())) {
    return var->Accept(*this);
  }
  throw CodeGenException("invalid declaration statement");
}

auto BaseIRVisitor::Codegen(const IfStmt &stmt) -> llvm::Value * {
  llvm::Function *parent_func = builder_->GetInsertBlock()->getParent();
  llvm::Type *ret_ty = parent_func->getReturnType();

  /// set condition expression
  llvm::Value *cond_val = stmt.cond_->Accept(*this);
  if (cond_val == nullptr) {
    throw CodeGenException("null condition expr for if-else statement");
  }
  llvm::Value *zero_val;
  llvm::Value *one_val;
  if (cond_val->getType() != llvm::Type::getInt1Ty(*context_)) {
    if (cond_val->getType() == llvm::Type::getInt64Ty(*context_)) {
      zero_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0);
      one_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 1);
      cond_val = builder_->CreateICmpNE(cond_val, zero_val);
    } else if (cond_val->getType() == llvm::Type::getInt64Ty(*context_)) {
      zero_val = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context_), 0);
      one_val = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context_), 1);
      cond_val = builder_->CreateFCmpONE(cond_val, zero_val);
    } else {
      throw CodeGenException("not implemented!");
    }
  }

  if (stmt.else_stmt_ != nullptr) {
    /// create basic blocks
    llvm::BasicBlock *then_b =
        llvm::BasicBlock::Create(*context_, "", parent_func);
    llvm::BasicBlock *else_b =
        llvm::BasicBlock::Create(*context_, "", parent_func);
    llvm::BasicBlock *merge_b =
        llvm::BasicBlock::Create(*context_, "", parent_func);
    builder_->CreateCondBr(cond_val, then_b, else_b);

    /// then block
    builder_->SetInsertPoint(then_b);
    llvm::Value *then_val = stmt.then_stmt_->Accept(*this);
    if (then_val == nullptr ||
        then_val->getType() == llvm::Type::getVoidTy(*context_)) {
      then_val = zero_val;
    }
    builder_->CreateBr(merge_b);
    then_b = builder_->GetInsertBlock();

    /// else block
    parent_func->insert(parent_func->end(), else_b);
    builder_->SetInsertPoint(else_b);
    llvm::Value *else_val = nullptr;
    else_val = stmt.else_stmt_->Accept(*this);
    if (else_val == nullptr ||
        else_val->getType() == llvm::Type::getVoidTy(*context_)) {
      else_val = one_val;
    }
    builder_->CreateBr(merge_b);
    else_b = builder_->GetInsertBlock();

    /// merge block
    parent_func->insert(parent_func->end(), merge_b);
    builder_->SetInsertPoint(merge_b);
    llvm::PHINode *pn = builder_->CreatePHI(ret_ty, 2);
    pn->addIncoming(then_val, then_b);
    pn->addIncoming(else_val, else_b);
    return pn;
  }
  /**
   * if there is no else-stmt
   */

  /// create basic blocks
  llvm::BasicBlock *then_b =
      llvm::BasicBlock::Create(*context_, "then", parent_func);
  llvm::BasicBlock *after_b =
      llvm::BasicBlock::Create(*context_, "after", parent_func);
  builder_->CreateCondBr(cond_val, then_b, after_b);

  /// then block
  builder_->SetInsertPoint(then_b);
  llvm::Value *then_val = stmt.then_stmt_->Accept(*this);
  if (then_val == nullptr ||
      then_val->getType() == llvm::Type::getVoidTy(*context_)) {
    then_val = zero_val;
  }

  /// create return in if-stmt
  if (parent_func->getReturnType()->isVoidTy()) {
    builder_->CreateRetVoid();
  } else {
    /// if function does not have a terminator, create a return instruction
    if (then_val == nullptr ||
        (then_val != nullptr &&
         then_val->getType() == llvm::Type::getVoidTy(*context_))) {
      if (parent_func->getReturnType() == llvm::Type::getInt64Ty(*context_)) {
        then_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0);
      } else {
        then_val = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context_), 0);
      }
    }
    builder_->CreateRet(then_val);
  }

  /// after block
  builder_->SetInsertPoint(after_b);
  return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*context_));
}

auto BaseIRVisitor::Codegen(const WhileStmt &stmt) -> llvm::Value * {
  llvm::Function *parent_func = builder_->GetInsertBlock()->getParent();
  llvm::Type *ret_ty = parent_func->getReturnType();

  /// create basic blocks
  llvm::BasicBlock *cond_b =
      llvm::BasicBlock::Create(*context_, "", parent_func);
  llvm::BasicBlock *body_b =
      llvm::BasicBlock::Create(*context_, "", parent_func);
  llvm::BasicBlock *exit_b =
      llvm::BasicBlock::Create(*context_, "", parent_func);

  /// cond block
  builder_->CreateBr(cond_b);
  builder_->SetInsertPoint(cond_b);

  /// set condition expression
  llvm::Value *cond_val = stmt.cond_->Accept(*this);
  if (cond_val == nullptr) {
    throw CodeGenException("null condition expr for if-else statement");
  }
  llvm::Value *zero_val;
  if (cond_val->getType() != llvm::Type::getInt1Ty(*context_)) {
    if (cond_val->getType() == llvm::Type::getInt64Ty(*context_)) {
      zero_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0);
      cond_val = builder_->CreateICmpNE(cond_val, zero_val);
    } else if (cond_val->getType() == llvm::Type::getInt64Ty(*context_)) {
      zero_val = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context_), 0);
      cond_val = builder_->CreateFCmpONE(cond_val, zero_val);
    } else {
      throw CodeGenException("not implemented!");
    }
  }

  /// set condition branch
  builder_->CreateCondBr(cond_val, body_b, exit_b);

  /// then block
  builder_->SetInsertPoint(body_b);
  if (stmt.stmt_ != nullptr) {
    stmt.stmt_->Accept(*this);
  }
  builder_->CreateBr(cond_b);

  /// exit block
  builder_->SetInsertPoint(exit_b);
  return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*context_));
}

auto BaseIRVisitor::Codegen(const ForStmt &stmt) -> llvm::Value * {
  llvm::Value *init = stmt.init_->Accept(*this);
  llvm::Function *parent_func = builder_->GetInsertBlock()->getParent();
  llvm::Type *ret_ty = parent_func->getReturnType();

  /// init
  stmt.init_->Accept(*this);

  /// condition block
  llvm::BasicBlock *cond_b =
      llvm::BasicBlock::Create(*context_, "", parent_func);
  builder_->CreateBr(cond_b);
  builder_->SetInsertPoint(cond_b);
  /// condition check
  llvm::Value *cmp = stmt.cond_->Accept(*this);
  llvm::BasicBlock *body_b =
      llvm::BasicBlock::Create(*context_, "", parent_func);
  llvm::BasicBlock *exit_b =
      llvm::BasicBlock::Create(*context_, "", parent_func);
  builder_->CreateCondBr(cmp, body_b, exit_b);

  /// loop body
  builder_->SetInsertPoint(body_b);
  stmt.body_->Accept(*this);
  /// update loop
  stmt.update_->Accept(*this);
  builder_->CreateBr(cond_b);
  /// exit
  builder_->SetInsertPoint(exit_b);
  var_env_.erase(stmt.init_->decl_->GetName());
  return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*context_));
}

auto BaseIRVisitor::Codegen(const ReturnStmt &stmt) -> llvm::Value * {
  if (stmt.expr_ != nullptr) {
    llvm::Value *ret_val = stmt.expr_->Accept(*this);
    return ret_val;
  }
  return nullptr;
}

/**
 * Decl
 */

auto BaseIRVisitor::Codegen(const ParmVarDecl &decl) -> llvm::Type * {
  llvm::Type *param_ty;
  if (decl.type_.starts_with("i64")) {
    param_ty = llvm::Type::getInt64Ty(*context_);
  } else if (decl.type_.starts_with("f64")) {
    param_ty = llvm::Type::getDoubleTy(*context_);
  }
  return param_ty;
}

auto BaseIRVisitor::Codegen(const FunctionDecl &decl) -> llvm::Function * {
  llvm::Function *func = GetFunction(decl);
  if (decl.kind_ == EXTERN_FUNC) {
    return func;
  }

  /// function entry point
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context_, "", func);
  builder_->SetInsertPoint(bb);

  /// clear local variable table
  ClearVarEnv();
  for (auto &param : func->args()) {
    size_t idx = param.getArgNo();
    std::string param_name = decl.proto_->params_[idx]->name_;
    llvm::Type *type = func->getFunctionType()->getParamType(idx);
    var_env_[param_name] = builder_->CreateAlloca(type, nullptr);
    builder_->CreateStore(&param, var_env_[param_name]);
  }

  /// return type
  llvm::Value *ret_val = nullptr;
  if (decl.body_ != nullptr) {
    if (llvm::Value *ret = decl.body_->Accept(*this)) {
      ret_val = ret;
    }
  }
  /// create void return if function type is void
  if (func->getReturnType()->isVoidTy()) {
    builder_->CreateRetVoid();
  } else {
    /// if function does not have a terminator, create a return instruction
    if (ret_val == nullptr ||
        (ret_val != nullptr &&
         ret_val->getType() == llvm::Type::getVoidTy(*context_))) {
      if (func->getReturnType() == llvm::Type::getInt64Ty(*context_)) {
        ret_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0);
      } else {
        ret_val = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context_), 0);
      }
    }
    builder_->CreateRet(ret_val);
  }
  return func;
}

/**
 * CompilerIRVisitor
 */

void CompilerIRVisitor::PrintGlobalVarEnv() {
  std::cout << "\033[1;32mGlobalVarEnv:\n";
  for (auto &[first, second] : global_var_env_) {
    std::string value_str;
    llvm::raw_string_ostream ros(value_str);
    if (second != nullptr) {
      second->print(ros);
    } else {
      ros << "<null>";
    }
    std::cout << makeString("  <var> '{}': {}\n", first, value_str);
  }
  std::cout << "\033[0m\n";
}

CompilerIRVisitor::CompilerIRVisitor() {
  context_ = std::make_unique<llvm::LLVMContext>();
  builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
  module_ = std::make_unique<llvm::Module>("toycc", *context_);
  module_->setTargetTriple(llvm::sys::getDefaultTargetTriple());
}

void CompilerIRVisitor::SetModuleID(std::string &name) {
  module_->setModuleIdentifier(name);
  module_->setSourceFileName(name);
}

auto CompilerIRVisitor::Codegen(const DeclRefExpr &expr) -> llvm::Value * {
  std::string var_name = expr.decl_->GetName();
  llvm::AllocaInst *id = var_env_[var_name];
  if (id != nullptr) {
    auto *id_val = builder_->CreateLoad(id->getAllocatedType(), id);
    if (id_val == nullptr) {
      throw CodeGenException(makeString("identifier '{}' not load", var_name));
    }
    return id_val;
  }
  llvm::GlobalVariable *gid = global_var_env_[var_name];
  if (gid != nullptr) {
    auto *id_val = builder_->CreateLoad(gid->getValueType(), gid);
    if (id_val == nullptr) {
      throw CodeGenException(makeString("identifier '{}' not load", var_name));
    }
    return id_val;
  }
  throw CodeGenException(makeString("identifier '{}' not found", var_name));
}

auto CompilerIRVisitor::Codegen(const CallExpr &expr) -> llvm::Value * {
  llvm::Function *callee = module_->getFunction(expr.callee_->decl_->GetName());
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

auto CompilerIRVisitor::Codegen(const UnaryOperator &expr) -> llvm::Value * {
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
      ptr = global_var_env_[var_name];
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

auto CompilerIRVisitor::Codegen(const BinaryOperator &expr) -> llvm::Value * {
  llvm::Value *l;
  llvm::Value *r;

  if (expr.op_.type_ == EQUAL) {
    if (auto *left = dynamic_cast<DeclRefExpr *>(expr.left_.get())) {
      std::string var_name = left->decl_->GetName();
      l = var_env_[var_name];
      if (l == nullptr) {
        l = global_var_env_[var_name];
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
    case GT:
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
    case GT:
      return builder_->CreateFCmpOGT(l, r);
    default:
      break;
    }
  }
  throw CodeGenException("[BinaryOperator] unimplemented binary operation");
}

auto CompilerIRVisitor::Codegen(const VarDecl &decl) -> llvm::Value * {
  llvm::Type *var_ty =
      (decl.type_ == "i64" ? builder_->getInt64Ty() : builder_->getDoubleTy());
  llvm::Constant *initializer =
      (decl.init_ != nullptr
           ? reinterpret_cast<llvm::Constant *>(decl.init_->Accept(*this))
           : nullptr);

  if (decl.scope_ == GLOBAL) {
    auto *var = new llvm::GlobalVariable(*module_, var_ty, false,
                                         llvm::GlobalVariable::ExternalLinkage,
                                         initializer, decl.name_);
    global_var_env_[decl.name_] = var;
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
 * TranslationUnitDecl
 */

void CompilerIRVisitor::Codegen(const TranslationUnitDecl &decl) {
  for (auto &d : decl.decls_) {
    if (auto *var_decl = dynamic_cast<VarDecl *>(d.get())) {
      var_decl->Accept(*this);
    } else if (auto *func_decl = dynamic_cast<FunctionDecl *>(d.get())) {
      if (func_decl->GetKind() != DECLARATION) {
        func_decl->Accept(*this);
      }
    } else {
      throw CodeGenException("[TranslationUnitDecl] unsupported declaration");
    }
  }

  /// remove not used extern functions
  auto function_list = &module_->getFunctionList();
  for (auto it = function_list->begin(), end = function_list->end();
       it != end;) {
    auto &f = *it++;
    if (f.isDeclaration() && f.users().empty()) {
      // debug("remove extern function: {} (not used)", f.getName());
      f.eraseFromParent();
    }
  }
}

} // namespace toyc
