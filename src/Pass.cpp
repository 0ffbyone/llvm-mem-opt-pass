#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <utility>
#include <variant>
#include <vector>

#include "MemOpt.h"

using namespace llvm;

namespace {
struct MemOpt : PassInfoMixin<MemOpt> {
  PreservedAnalyses run(Function& func, FunctionAnalysisManager&) {
    std::vector<CallInst*> allocs = memopt::findHeapAllocations(func);

    std::vector<std::variant<BranchInst*, SwitchInst*>>
        weightedBranch = memopt::findWeightedBranches(func);

    BasicBlock* needOptBlock;

    std::vector<std::pair<CallInst*, BasicBlock*>> canBeOptimizedVec;

    for (const auto& alloc : allocs) {
        errs() << "alloc "<< alloc << '\n';
        for (const std::variant<BranchInst*, SwitchInst*>& branch : weightedBranch) {
            needOptBlock = std::visit([&](auto&& arg)
                    { return memopt::needOptimization(func, arg, alloc); }, branch);
            if (needOptBlock) {
                auto pair = std::make_pair(alloc, needOptBlock);
                canBeOptimizedVec.push_back(pair);
            }
        }
    }

    errs() << ((canBeOptimizedVec.size() > 0)? "можно оптимизировать\n": "нельзя оптимизировать\n");


    for (const auto& el : canBeOptimizedVec) {
        memopt::moveAllocInsideWeightedBlock(el.first, *el.second);
    }

    if (canBeOptimizedVec.size() > 0) {
        errs() << canBeOptimizedVec.size() << '\n';
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
