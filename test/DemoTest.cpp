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
  Module module("foo", context);
  IRBuilder<> builder(context);

  // Define function signature
  FunctionType *funcType = FunctionType::get(builder.getInt32Ty(), false);
  Function *fooFunc =
      Function::Create(funcType, Function::ExternalLinkage, "foo", &module);
  BasicBlock *entryBlock = BasicBlock::Create(context, "entry", fooFunc);
  builder.SetInsertPoint(entryBlock);



  // Return the result
  // builder.CreateRet(sum);

  // Print LLVM IR to stdout
  module.print(outs(), nullptr);

  // Verify the generated IR
  verifyModule(module);
}
