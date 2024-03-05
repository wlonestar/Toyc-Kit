//! toyc just in time

#ifndef JIT_H
#define JIT_H

#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/Mangling.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Error.h>

#include <memory>

namespace toyc {

class ToycJIT {
private:
  std::unique_ptr<llvm::orc::ExecutionSession> executionSession;
  llvm::DataLayout dataLayout;
  llvm::orc::MangleAndInterner mangle;
  llvm::orc::RTDyldObjectLinkingLayer objectLayer;
  llvm::orc::IRCompileLayer compileLayer;
  llvm::orc::JITDylib &mainJd;

public:
  ToycJIT(std::unique_ptr<llvm::orc::ExecutionSession> _es,
          llvm::orc::JITTargetMachineBuilder _jtmb,
          llvm::DataLayout _dataLayout)
      : executionSession(std::move(_es)), dataLayout(std::move(_dataLayout)),
        mangle(*this->executionSession, this->dataLayout),
        objectLayer(
            *this->executionSession,
            []() { return std::make_unique<llvm::SectionMemoryManager>(); }),
        compileLayer(*this->executionSession, objectLayer,
                     std::make_unique<llvm::orc::ConcurrentIRCompiler>(
                         std::move(_jtmb))),
        mainJd(this->executionSession->createBareJITDylib("<main>")) {
    mainJd.addGenerator(llvm::cantFail(
        llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            dataLayout.getGlobalPrefix())));
    if (_jtmb.getTargetTriple().isOSBinFormatCOFF()) {
      objectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
      objectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
  }

  ~ToycJIT() {
    if (auto err = executionSession->endSession()) {
      executionSession->reportError(std::move(err));
    }
  }

  static llvm::Expected<std::unique_ptr<ToycJIT>> create() {
    auto epc = llvm::orc::SelfExecutorProcessControl::Create();
    if (!epc) {
      return epc.takeError();
    }

    auto es = std::make_unique<llvm::orc::ExecutionSession>(std::move(*epc));
    llvm::orc::JITTargetMachineBuilder jtmb(
        es->getExecutorProcessControl().getTargetTriple());

    auto dl = jtmb.getDefaultDataLayoutForTarget();
    if (!dl) {
      return dl.takeError();
    }

    return std::make_unique<ToycJIT>(std::move(es), std::move(jtmb),
                                     std::move(*dl));
  }

  const llvm::DataLayout &getDataLayout() const { return dataLayout; }
  llvm::orc::JITDylib &getMainJITDylib() { return mainJd; }

  llvm::Error addModule(llvm::orc::ThreadSafeModule tsm,
                        llvm::orc::ResourceTrackerSP rt = nullptr) {
    if (!rt) {
      rt = mainJd.getDefaultResourceTracker();
    }
    return compileLayer.add(rt, std::move(tsm));
  }

  llvm::Expected<llvm::JITEvaluatedSymbol> lookup(std::string name) {
    return executionSession->lookup({&mainJd}, mangle(name));
  }
};

} // namespace toyc

#endif
