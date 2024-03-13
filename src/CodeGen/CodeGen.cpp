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

void BaseIRVisitor::printVarEnv() {
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

void BaseIRVisitor::dump(llvm::raw_ostream &os) {
  module->print(os, nullptr, false, false);
}

bool BaseIRVisitor::verifyModule(llvm::raw_ostream &os) {
  std::string info;
  llvm::raw_string_ostream ros(info);
  llvm::verifyModule(*module, &ros, nullptr);
  if (info.size() == 0) {
    return true;
  } else {
    os << "\033[1;31mverify:\n" << info << "\033[0m\n";
    return false;
  }
}

llvm::Function *BaseIRVisitor::getFunction(const FunctionDecl &decl) {
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

llvm::Value *BaseIRVisitor::codegen(const IntegerLiteral &expr) {
  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), expr.value);
}

llvm::Value *BaseIRVisitor::codegen(const FloatingLiteral &expr) {
  return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), expr.value);
}

llvm::Value *BaseIRVisitor::codegen(const ImplicitCastExpr &expr) {
  if (expr.type == expr.expr->getType()) {
    return expr.expr->accept(*this);
  } else {
    auto *value = expr.expr->accept(*this);
    if (expr.type == "f64" && expr.expr->getType() == "i64") {
      return builder->CreateSIToFP(value, llvm::Type::getDoubleTy(*context));
    } else if (expr.type == "i64" && expr.expr->getType() == "f64") {
      return builder->CreateFPToSI(value, llvm::Type::getInt64Ty(*context));
    } else {
      throw CodeGenException("not implemented!");
    }
  }
}

llvm::Value *BaseIRVisitor::codegen(const CastExpr &expr) {
  if (expr.type == expr.expr->getType()) {
    return expr.expr->accept(*this);
  } else {
    auto *value = expr.expr->accept(*this);
    if (expr.type == "f64" && expr.expr->getType() == "i64") {
      return builder->CreateSIToFP(value, llvm::Type::getDoubleTy(*context));
    } else if (expr.type == "i64" && expr.expr->getType() == "f64") {
      return builder->CreateFPToSI(value, llvm::Type::getInt64Ty(*context));
    } else {
      throw CodeGenException("not implemented!");
    }
  }
}

llvm::Value *BaseIRVisitor::codegen(const ParenExpr &expr) {
  return expr.expr->accept(*this);
}

llvm::Value *BaseIRVisitor::codegen(const UnaryOperator &expr) {
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
    var = varEnv[_left->decl->getName()];
  }

  switch (expr.op.type) {
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

/**
 * Stmt
 */

llvm::Value *BaseIRVisitor::codegen(const CompoundStmt &stmt) {
  llvm::Value *retVal = nullptr;
  for (auto &stmt : stmt.stmts) {
    if (auto ret = stmt->accept(*this)) {
      retVal = ret;
    }
  }
  return retVal;
}

llvm::Value *BaseIRVisitor::codegen(const ExprStmt &stmt) {
  return stmt.expr->accept(*this);
}

llvm::Value *BaseIRVisitor::codegen(const DeclStmt &stmt) {
  if (auto var = dynamic_cast<VarDecl *>(stmt.decl.get())) {
    return var->accept(*this);
  }
  throw CodeGenException("invalid declaration statement");
}

llvm::Value *BaseIRVisitor::codegen(const IfStmt &stmt) {
  llvm::Function *parentFunc = builder->GetInsertBlock()->getParent();
  llvm::Type *retTy = parentFunc->getReturnType();

  /// set condition expression
  llvm::Value *condVal = stmt.cond->accept(*this);
  if (condVal == nullptr) {
    throw CodeGenException("null condition expr for if-else statement");
  }
  llvm::Value *zeroVal, *oneVal;
  if (condVal->getType() != llvm::Type::getInt1Ty(*context)) {
    if (condVal->getType() == llvm::Type::getInt64Ty(*context)) {
      zeroVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0);
      oneVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1);
      condVal = builder->CreateICmpNE(condVal, zeroVal);
    } else if (condVal->getType() == llvm::Type::getInt64Ty(*context)) {
      zeroVal = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), 0);
      oneVal = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), 1);
      condVal = builder->CreateFCmpONE(condVal, zeroVal);
    } else {
      throw CodeGenException("not implemented!");
    }
  }

  if (stmt.elseStmt != nullptr) {
    /// create basic blocks
    llvm::BasicBlock *thenB =
        llvm::BasicBlock::Create(*context, "", parentFunc);
    llvm::BasicBlock *elseB =
        llvm::BasicBlock::Create(*context, "", parentFunc);
    llvm::BasicBlock *mergeB =
        llvm::BasicBlock::Create(*context, "", parentFunc);
    builder->CreateCondBr(condVal, thenB, elseB);

    /// then block
    builder->SetInsertPoint(thenB);
    llvm::Value *thenVal = stmt.thenStmt->accept(*this);
    if (thenVal == nullptr ||
        thenVal->getType() == llvm::Type::getVoidTy(*context)) {
      thenVal = zeroVal;
    }
    builder->CreateBr(mergeB);
    thenB = builder->GetInsertBlock();

    /// else block
    parentFunc->insert(parentFunc->end(), elseB);
    builder->SetInsertPoint(elseB);
    llvm::Value *elseVal = nullptr;
    elseVal = stmt.elseStmt->accept(*this);
    if (elseVal == nullptr ||
        elseVal->getType() == llvm::Type::getVoidTy(*context)) {
      elseVal = oneVal;
    }
    builder->CreateBr(mergeB);
    elseB = builder->GetInsertBlock();

    /// merge block
    parentFunc->insert(parentFunc->end(), mergeB);
    builder->SetInsertPoint(mergeB);
    llvm::PHINode *pn = builder->CreatePHI(retTy, 2);
    pn->addIncoming(thenVal, thenB);
    pn->addIncoming(elseVal, elseB);
    return pn;
  } else {
    /**
     * if there is no else-stmt
     */

    /// create basic blocks
    llvm::BasicBlock *thenB =
        llvm::BasicBlock::Create(*context, "then", parentFunc);
    llvm::BasicBlock *afterB =
        llvm::BasicBlock::Create(*context, "after", parentFunc);
    builder->CreateCondBr(condVal, thenB, afterB);

    /// then block
    builder->SetInsertPoint(thenB);
    llvm::Value *thenVal = stmt.thenStmt->accept(*this);
    if (thenVal == nullptr ||
        thenVal->getType() == llvm::Type::getVoidTy(*context)) {
      thenVal = zeroVal;
    }

    /// create return in if-stmt
    if (parentFunc->getReturnType()->isVoidTy()) {
      builder->CreateRetVoid();
    } else {
      /// if function does not have a terminator, create a return instruction
      if (thenVal == nullptr ||
          (thenVal != nullptr &&
           thenVal->getType() == llvm::Type::getVoidTy(*context))) {
        if (parentFunc->getReturnType() == llvm::Type::getInt64Ty(*context)) {
          thenVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0);
        } else {
          thenVal = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), 0);
        }
      }
      builder->CreateRet(thenVal);
    }

    /// after block
    builder->SetInsertPoint(afterB);
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*context));
  }
}

llvm::Value *BaseIRVisitor::codegen(const WhileStmt &stmt) {
  llvm::Function *parentFunc = builder->GetInsertBlock()->getParent();
  llvm::Type *retTy = parentFunc->getReturnType();

  /// create basic blocks
  llvm::BasicBlock *condB = llvm::BasicBlock::Create(*context, "", parentFunc);
  llvm::BasicBlock *bodyB = llvm::BasicBlock::Create(*context, "", parentFunc);
  llvm::BasicBlock *exitB = llvm::BasicBlock::Create(*context, "", parentFunc);

  /// cond block
  builder->CreateBr(condB);
  builder->SetInsertPoint(condB);

  /// set condition expression
  llvm::Value *condVal = stmt.cond->accept(*this);
  if (condVal == nullptr) {
    throw CodeGenException("null condition expr for if-else statement");
  }
  llvm::Value *zeroVal;
  if (condVal->getType() != llvm::Type::getInt1Ty(*context)) {
    if (condVal->getType() == llvm::Type::getInt64Ty(*context)) {
      zeroVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0);
      condVal = builder->CreateICmpNE(condVal, zeroVal);
    } else if (condVal->getType() == llvm::Type::getInt64Ty(*context)) {
      zeroVal = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), 0);
      condVal = builder->CreateFCmpONE(condVal, zeroVal);
    } else {
      throw CodeGenException("not implemented!");
    }
  }

  /// set condition branch
  builder->CreateCondBr(condVal, bodyB, exitB);

  /// then block
  builder->SetInsertPoint(bodyB);
  if (stmt.stmt != nullptr) {
    stmt.stmt->accept(*this);
  }
  builder->CreateBr(condB);

  /// exit block
  builder->SetInsertPoint(exitB);
  return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*context));
}

llvm::Value *BaseIRVisitor::codegen(const ForStmt &stmt) {
  llvm::Value *init = stmt.init->accept(*this);
  llvm::Function *parentFunc = builder->GetInsertBlock()->getParent();
  llvm::Type *retTy = parentFunc->getReturnType();

  /// init
  stmt.init->accept(*this);

  /// condition block
  llvm::BasicBlock *condB = llvm::BasicBlock::Create(*context, "", parentFunc);
  builder->CreateBr(condB);
  builder->SetInsertPoint(condB);
  /// condition check
  auto cmp = stmt.cond->accept(*this);
  llvm::BasicBlock *bodyB = llvm::BasicBlock::Create(*context, "", parentFunc);
  llvm::BasicBlock *exitB = llvm::BasicBlock::Create(*context, "", parentFunc);
  builder->CreateCondBr(cmp, bodyB, exitB);

  /// loop body
  builder->SetInsertPoint(bodyB);
  stmt.body->accept(*this);
  /// update loop
  stmt.update->accept(*this);
  builder->CreateBr(condB);
  /// exit
  builder->SetInsertPoint(exitB);
  varEnv.erase(stmt.init->decl->getName());
  return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*context));
}

llvm::Value *BaseIRVisitor::codegen(const ReturnStmt &stmt) {
  if (stmt.expr != nullptr) {
    llvm::Value *retVal = stmt.expr->accept(*this);
    return retVal;
  } else {
    return nullptr;
  }
}

/**
 * Decl
 */

llvm::Type *BaseIRVisitor::codegen(const ParmVarDecl &decl) {
  llvm::Type *paramTy;
  if (decl.type.starts_with("i64")) {
    paramTy = llvm::Type::getInt64Ty(*context);
  } else if (decl.type.starts_with("f64")) {
    paramTy = llvm::Type::getDoubleTy(*context);
  }
  return paramTy;
}

llvm::Function *BaseIRVisitor::codegen(const FunctionDecl &decl) {
  llvm::Function *func = getFunction(decl);
  if (decl.kind == EXTERN_FUNC) {
    return func;
  }

  /// function entry point
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "", func);
  builder->SetInsertPoint(bb);

  /// clear local variable table
  clearVarEnv();
  for (auto &param : func->args()) {
    size_t idx = param.getArgNo();
    std::string paramName = decl.proto->params[idx]->name;
    llvm::Type *type = func->getFunctionType()->getParamType(idx);
    varEnv[paramName] = builder->CreateAlloca(type, nullptr);
    builder->CreateStore(&param, varEnv[paramName]);
  }

  /// return type
  llvm::Value *retVal = nullptr;
  if (decl.body != nullptr) {
    if (auto ret = decl.body->accept(*this)) {
      retVal = ret;
    }
  }
  /// create void return if function type is void
  if (func->getReturnType()->isVoidTy()) {
    builder->CreateRetVoid();
  } else {
    /// if function does not have a terminator, create a return instruction
    if (retVal == nullptr ||
        (retVal != nullptr &&
         retVal->getType() == llvm::Type::getVoidTy(*context))) {
      if (func->getReturnType() == llvm::Type::getInt64Ty(*context)) {
        retVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0);
      } else {
        retVal = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), 0);
      }
    }
    builder->CreateRet(retVal);
  }
  return func;
}

/**
 * CompilerIRVisitor
 */

void CompilerIRVisitor::printGlobalVarEnv() {
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

CompilerIRVisitor::CompilerIRVisitor() {
  context = std::make_unique<llvm::LLVMContext>();
  builder = std::unique_ptr<llvm::IRBuilder<>>(new llvm::IRBuilder<>(*context));
  module = std::make_unique<llvm::Module>("toycc", *context);
  module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
}

void CompilerIRVisitor::setModuleID(std::string &name) {
  module->setModuleIdentifier(name);
  module->setSourceFileName(name);
}

llvm::Value *CompilerIRVisitor::codegen(const DeclRefExpr &expr) {
  std::string varName = expr.decl->getName();
  auto *id = varEnv[varName];
  if (id != nullptr) {
    auto *idVal = builder->CreateLoad(id->getAllocatedType(), id);
    if (idVal == nullptr) {
      throw CodeGenException(fstr("identifier '{}' not load", varName));
    }
    return idVal;
  }
  auto *gid = globalVarEnv[varName];
  if (gid != nullptr) {
    auto *idVal = builder->CreateLoad(gid->getValueType(), gid);
    if (idVal == nullptr) {
      throw CodeGenException(fstr("identifier '{}' not load", varName));
    }
    return idVal;
  }
  throw CodeGenException(fstr("identifier '{}' not found", varName));
}

llvm::Value *CompilerIRVisitor::codegen(const CallExpr &expr) {
  llvm::Function *callee = module->getFunction(expr.callee->decl->getName());
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

llvm::Value *CompilerIRVisitor::codegen(const BinaryOperator &expr) {
  llvm::Value *l, *r;

  if (expr.op.type == EQUAL) {
    if (auto *_left = dynamic_cast<DeclRefExpr *>(expr.left.get())) {
      std::string varName = _left->decl->getName();
      l = varEnv[varName];
      if (l == nullptr) {
        l = globalVarEnv[varName];
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

llvm::Value *CompilerIRVisitor::codegen(const VarDecl &decl) {
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
 * TranslationUnitDecl
 */

void CompilerIRVisitor::codegen(const TranslationUnitDecl &decl) {
  for (auto &d : decl.decls) {
    if (auto varDecl = dynamic_cast<VarDecl *>(d.get())) {
      varDecl->accept(*this);
    } else if (auto funcDecl = dynamic_cast<FunctionDecl *>(d.get())) {
      if (funcDecl->getKind() != DECLARATION) {
        funcDecl->accept(*this);
      }
    } else {
      throw CodeGenException("[TranslationUnitDecl] unsupported declaration");
    }
  }

  /// remove not used extern functions
  auto functionList = &module->getFunctionList();
  for (auto it = functionList->begin(), end = functionList->end(); it != end;) {
    auto &f = *it++;
    if (f.isDeclaration() && f.users().empty()) {
      // debug("remove extern function: {} (not used)", f.getName());
      f.eraseFromParent();
    }
  }
}

} // namespace toyc
