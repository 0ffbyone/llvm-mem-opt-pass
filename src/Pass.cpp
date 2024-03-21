#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <type_traits>
#include <algorithm>
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

    std::set<std::pair<CallInst*, BasicBlock*>> canBeOptimizedSet;

    // I should check that brances are different
    for (auto& alloc : allocs) {
        for (std::variant<BranchInst*, SwitchInst*>& branch : weightedBranch) {
            needOptBlock = std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same<T, BranchInst*>::value) {
                        return memopt::needOptimization<BranchInst>(arg, alloc);
                    } else {
                        return memopt::needOptimization<SwitchInst>(arg, alloc);
                    }
                    }, branch);

            if (needOptBlock) {
                auto pair = std::make_pair(alloc, needOptBlock);
                canBeOptimizedSet.insert(pair);
            }
        }
    }

    std::cerr << ((canBeOptimizedSet.size() > 0)? "YES": "NO");

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
