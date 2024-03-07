// ! code generation implementation

#include <AST/AST.h>
#include <CodeGen/InterpreterCodeGen.h>
#include <Lexer/Token.h>
#include <Parser/InterpreterParser.h>
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
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace toyc {

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" DLLEXPORT void printI64(int64_t x) { fprintf(stderr, "%ld\n", x); }

extern "C" DLLEXPORT void printF64(double x) { fprintf(stderr, "%lf\n", x); }

InterpreterCodegenVisitor::InterpreterCodegenVisitor() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  JIT = ExitOnErr(ToycJIT::create());

  module->setDataLayout(JIT->getDataLayout());

  // Create new pass and analysis managers.
  FPM = std::make_unique<llvm::FunctionPassManager>();
  LAM = std::make_unique<llvm::LoopAnalysisManager>();
  FAM = std::make_unique<llvm::FunctionAnalysisManager>();
  CGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
  MAM = std::make_unique<llvm::ModuleAnalysisManager>();
  PIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
  SI = std::make_unique<llvm::StandardInstrumentations>(*context, true);
  SI->registerCallbacks(*PIC, FAM.get());

  // Add transform passes.
  FPM->addPass(llvm::InstCombinePass());
  FPM->addPass(llvm::ReassociatePass());
  FPM->addPass(llvm::GVNPass());
  FPM->addPass(llvm::SimplifyCFGPass());

  // Register analysis passes used in these transform passes.
  llvm::PassBuilder PB;
  PB.registerModuleAnalyses(*MAM);
  PB.registerFunctionAnalyses(*FAM);
  PB.crossRegisterProxies(*LAM, *FAM, *CGAM, *MAM);
}

/**
 * Decl
 */

llvm::Value *InterpreterCodegenVisitor::codegen(const VarDecl &decl) {
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

// llvm::Function *InterpreterCodegenVisitor::codegen(const FunctionDecl &decl)
// {
//   llvm::Function *func = codegenFuncTy(decl);
//   if (decl.kind == EXTERN_FUNC) {
//     return func;
//   }

//   /// function entry point
//   llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "", func);
//   builder->SetInsertPoint(bb);

//   /// clear local variable table
//   clearVarEnv();
//   for (auto &param : func->args()) {
//     size_t idx = param.getArgNo();
//     std::string paramName = decl.params[idx]->name;
//     llvm::Type *type = func->getFunctionType()->getParamType(idx);
//     varEnv[paramName] = builder->CreateAlloca(type, nullptr);
//     builder->CreateStore(&param, varEnv[paramName]);
//   }

//   llvm::Value *retVal = nullptr;
//   if (decl.body != nullptr) {
//     if (auto ret = decl.body->accept(*this)) {
//       retVal = ret;
//     }
//   }

//   /// create void return if function type is void
//   if (func->getReturnType()->isVoidTy()) {
//     builder->CreateRetVoid();
//   } else {
//     /// if function does not have a terminator, create a return instruction
//     if (retVal == nullptr ||
//         (retVal != nullptr &&
//          retVal->getType() == llvm::Type::getVoidTy(*context))) {
//       if (func->getReturnType() == llvm::Type::getInt64Ty(*context)) {
//         retVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0);
//       } else {
//         retVal = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), 0);
//       }
//     }
//     builder->CreateRet(retVal);
//   }
//   return func;
// }

/**
 * TranslationUnitDecl
 */

void InterpreterCodegenVisitor::codegen(const TranslationUnitDecl &decl) {
  for (auto &d : decl.decls) {
    if (auto varDecl = dynamic_cast<VarDecl *>(d.get())) {
      varDecl->accept(*this);
    } else if (auto funcDecl = dynamic_cast<FunctionDecl *>(d.get())) {
      if (funcDecl->getKind() != DECLARATION) {
        auto *ir = funcDecl->accept(*this);
      }
    } else {
      throw CodeGenException("[TranslationUnitDecl] unsupported declaration");
    }
  }
}

} // namespace toyc
