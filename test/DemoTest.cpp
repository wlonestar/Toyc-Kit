#include <boost/test/tools/old/interface.hpp>
#define BOOST_TEST_MODULE DemoTest
#include <boost/test/unit_test.hpp>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

BOOST_AUTO_TEST_CASE(demo_test) {

  LLVMContext context;
  Module module("example", context);
  IRBuilder<> builder(context);

  // Define function signature: int foo()
  FunctionType *functionType =
      FunctionType::get(Type::getInt32Ty(context), false);
  Function *fooFunction =
      Function::Create(functionType, Function::ExternalLinkage, "foo", module);
  BasicBlock *entryBlock = BasicBlock::Create(context, "", fooFunction);
  builder.SetInsertPoint(entryBlock);

  // Create memory allocations for variables a, b, and c
  AllocaInst *a = builder.CreateAlloca(Type::getInt32Ty(context), nullptr);
  AllocaInst *b = builder.CreateAlloca(Type::getInt32Ty(context), nullptr);
  AllocaInst *c = builder.CreateAlloca(Type::getInt32Ty(context), nullptr);

  // Store values into variables a and b
  builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), 1), a);
  builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), 2), b);

  // Perform addition: c = a + b
  Value *loadedA = builder.CreateLoad(a->getAllocatedType(), a);
  Value *loadedB = builder.CreateLoad(b->getAllocatedType(), b);
  Value *sum = builder.CreateAdd(loadedA, loadedB);
  builder.CreateStore(sum, c);

  // Load value of c and return it
  Value *loadedC = builder.CreateLoad(c->getAllocatedType(), c);
  builder.CreateRet(loadedC);

  // Print LLVM IR
  module.print(outs(), nullptr);

  // Verify the generated IR
  verifyModule(module);
}
