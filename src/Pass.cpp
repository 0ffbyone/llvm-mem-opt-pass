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
// пройтись по всем бренчам, найти именно ту, которая является unlikely
// если в ней используется память, то тогда Transformation pass
struct MemOpt : PassInfoMixin<MemOpt> {
  PreservedAnalyses run(Function& func, FunctionAnalysisManager&) {
    CallInst* alloc = memopt::findHeapAllocation(func);
    std::vector<std::variant<BranchInst*, SwitchInst*>>
        weightedBranch = memopt::findWeightedBranches(func);
    errs() << weightedBranch.size() << '\n';


    bool canBeOptimized = false;
    for (const std::variant<BranchInst*, SwitchInst*>& branch : weightedBranch) {
        //errs() << (memopt::needOptimization(branch, alloc)? "yes": "no") << '\n';
        bool needOpt = std::visit([alloc](auto&& arg)
                { return memopt::needOptimization(arg, alloc);}, branch);
        canBeOptimized += needOpt;
    }

    errs() << (canBeOptimized? "можно оптимизировать\n": "нельзя оптимизировать\n");


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
