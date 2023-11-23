#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <variant>

#include "MemOpt.h"

using namespace llvm;

namespace {
struct MemOpt : PassInfoMixin<MemOpt> {
  PreservedAnalyses run(Function& func, FunctionAnalysisManager&) {
    CallInst* alloc = memopt::findHeapAllocation(func);
    std::vector<std::variant<BranchInst*, SwitchInst*>>
        weightedBranch = memopt::findWeightedBranches(func);

    BasicBlock* needOptBlock;
    bool canBeOptimized = false;
    for (const std::variant<BranchInst*, SwitchInst*>& branch : weightedBranch) {
        needOptBlock = std::visit([&](auto&& arg)
                { return memopt::needOptimization(func, arg, alloc); }, branch);
        if (needOptBlock) {
            canBeOptimized += true;
        }
    }

    errs() << (canBeOptimized? "можно оптимизировать\n": "нельзя оптимизировать\n");

    if (canBeOptimized) {
        memopt::moveAllocInsideWeightedBlock(alloc, *needOptBlock);
        return PreservedAnalyses::none();
    }

    return PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

llvm::PassPluginLibraryInfo getMemOptPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MemOpt", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "mem-opt") {
                    FPM.addPass(MemOpt());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getMemOptPluginInfo();
}
