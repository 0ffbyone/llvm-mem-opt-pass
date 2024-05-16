#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <iterator>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <utility>
#include <variant>
#include <vector>
#include <iostream>

#include "MemOpt.h"

#define DEBUG


using namespace llvm;

namespace {
struct MemOpt : PassInfoMixin<MemOpt> {
PreservedAnalyses run(Function& func, FunctionAnalysisManager&) {
    std::vector<CallInst*> allocs = memopt::findHeapAllocations(func);

    std::vector<std::variant<BranchInst*, SwitchInst*>>
        weightedBranches = memopt::findWeightedBranches(func);

    std::set<BasicBlock*> unlikelyBlocks;
    for (std::variant<BranchInst*, SwitchInst*>& branch : weightedBranches) {
        std::set<BasicBlock*> currentUnlikelyBlocks = std::visit([](auto&& arg) {
                return memopt::findUnlikelyBlocks(arg); }, branch);
        unlikelyBlocks.insert(currentUnlikelyBlocks.begin(), currentUnlikelyBlocks.end());
    }
    if (unlikelyBlocks.size() == 0) {
        return PreservedAnalyses::all();
    }

    BasicBlock* needOptBlock;
    std::set<std::pair<CallInst*, BasicBlock*>> canBeOptimizedSet;

    for (auto& alloc : allocs) {
        //errs() << *alloc << '\n';
        needOptBlock = memopt::needOptimization(&func, unlikelyBlocks, alloc);
        if (needOptBlock) {
            auto pair = std::make_pair(alloc, needOptBlock);
            canBeOptimizedSet.insert(pair);
        }
        //for (std::variant<BranchInst*, SwitchInst*>& branch : weightedBranches) {
        //    needOptBlock = std::visit([&](auto&& arg) {
        //            return memopt::needOptimization(&func, arg, alloc); }, branch);

        //    if (needOptBlock) {
        //        auto pair = std::make_pair(alloc, needOptBlock);
        //        canBeOptimizedSet.insert(pair);
        //    }
        //}
    }
    #ifdef DEBUG
        std::cerr << ((canBeOptimizedSet.size() > 0)? "YES": "NO");
    #endif

    for (const auto& el : canBeOptimizedSet) {
        memopt::moveAllocInsideWeightedBlock(el.first, *el.second);
    }

    if (canBeOptimizedSet.size() > 0) {
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
