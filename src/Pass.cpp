#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <utility>
#include <variant>
#include <vector>
#include <iostream>

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

    // I should check that brances are different
    for (const auto& alloc : allocs) {
        for (const std::variant<BranchInst*, SwitchInst*>& branch : weightedBranch) {
            needOptBlock = std::visit([&](auto&& arg)
                    { return memopt::needOptimization(func, arg, alloc); }, branch);
            if (needOptBlock) {
                auto pair = std::make_pair(alloc, needOptBlock);
                canBeOptimizedVec.push_back(pair);
            }
        }
    }

    std::sort(canBeOptimizedVec.begin(), canBeOptimizedVec.end());
    auto last_unique = std::unique(canBeOptimizedVec.begin(),
            canBeOptimizedVec.end());
    canBeOptimizedVec.erase(last_unique, canBeOptimizedVec.end());

    std::cerr << ((canBeOptimizedVec.size() > 0)? "YES": "NO");


    for (const auto& el : canBeOptimizedVec) {
        memopt::moveAllocInsideWeightedBlock(el.first, *el.second);
    }

    if (canBeOptimizedVec.size() > 0) {
        //errs() << "size: "<< canBeOptimizedVec.size() << '\n';
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
