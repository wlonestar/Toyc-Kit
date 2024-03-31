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

#include <memory>

namespace toyc {

class ToycJIT {
private:
  std::unique_ptr<llvm::orc::ExecutionSession> execution_session_;
  llvm::DataLayout data_layout_;
  llvm::orc::MangleAndInterner mangle_;
  llvm::orc::RTDyldObjectLinkingLayer object_layer_;
  llvm::orc::IRCompileLayer compile_layer_;
  llvm::orc::JITDylib &mainlib_;

public:
  ToycJIT(std::unique_ptr<llvm::orc::ExecutionSession> _es,
          llvm::orc::JITTargetMachineBuilder _jtmb,
          const llvm::DataLayout &_dataLayout)
      : execution_session_(std::move(_es)), data_layout_(_dataLayout),
        mangle_(*this->execution_session_, this->data_layout_),
        object_layer_(
            *this->execution_session_,
            []() { return std::make_unique<llvm::SectionMemoryManager>(); }),
        compile_layer_(*this->execution_session_, object_layer_,
                       std::make_unique<llvm::orc::ConcurrentIRCompiler>(
                           std::move(_jtmb))),
        mainlib_(this->execution_session_->createBareJITDylib("<main>")) {
    mainlib_.addGenerator(llvm::cantFail(
        llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            data_layout_.getGlobalPrefix())));
    if (_jtmb.getTargetTriple().isOSBinFormatCOFF()) {
      object_layer_.setOverrideObjectFlagsWithResponsibilityFlags(true);
      object_layer_.setAutoClaimResponsibilityForObjectSymbols(true);
    }
  }

  ~ToycJIT() {
    if (llvm::Error err = execution_session_->endSession()) {
      execution_session_->reportError(std::move(err));
    }
  }

  static auto Create() -> llvm::Expected<std::unique_ptr<ToycJIT>> {
    auto epc = llvm::orc::SelfExecutorProcessControl::Create();
    if (!epc) {
      return epc.takeError();
    }

    auto es = std::make_unique<llvm::orc::ExecutionSession>(std::move(*epc));
    llvm::orc::JITTargetMachineBuilder jtmb(
        es->getExecutorProcessControl().getTargetTriple());

    llvm::Expected<llvm::DataLayout> dl = jtmb.getDefaultDataLayoutForTarget();
    if (!dl) {
      return dl.takeError();
    }

    return std::make_unique<ToycJIT>(std::move(es), std::move(jtmb),
                                     std::move(*dl));
  }

  auto GetDataLayout() const -> const llvm::DataLayout & {
    return data_layout_;
  }

  auto GetMainJITDylib() -> llvm::orc::JITDylib & { return mainlib_; }

  auto AddModule(llvm::orc::ThreadSafeModule tsm,
                 llvm::orc::ResourceTrackerSP rt = nullptr) -> llvm::Error {
    if (!rt) {
      rt = mainlib_.getDefaultResourceTracker();
    }
    return compile_layer_.add(rt, std::move(tsm));
  }

  auto Lookup(const std::string &name)
      -> llvm::Expected<llvm::JITEvaluatedSymbol> {
    return execution_session_->lookup({&mainlib_}, mangle_(name));
  }
};

} // namespace toyc

#endif
