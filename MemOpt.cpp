#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include <utility>
#include <vector>

using namespace llvm;
namespace {
CallInst* findHeapAllocation(Function& func) {

    errs() << "In " << func.getName() << '\n';

    for (BasicBlock& block : func) {
        for (Instruction& inst: block) {
            auto* callInst = dyn_cast<CallInst>(&inst);
            // Только CallInst
            if (callInst) {
                Function* calledFunc = callInst->getCalledFunction();
                AttributeList attributes =  calledFunc->getAttributes();
                auto isAllocFunc = attributes.hasFnAttr(Attribute::AllocSize);

                errs() << calledFunc->getName() << '\n';

                if (isAllocFunc) {
                    return callInst;
                }
            }
        }
    }
    return nullptr;
}


BranchInst* findWeightedBranch(Function& func) {
    for (BasicBlock& block: func) {
        for (Instruction& inst : block) {
            BranchInst* branchInst = dyn_cast<BranchInst>(&inst);

            // Только br инструкции
            if (branchInst) {
                MDNode* metaData = branchInst->getMetadata("prof");

                // Только br иструкции, которые потенциально могут быть с весом
                if (branchInst->isConditional() && metaData) {
                    uint64_t branchWeight = 0;
                    branchInst->extractProfTotalWeight(branchWeight);
                    if (branchWeight >= 2000) {
                        return branchInst;
                        //branchInst->getSuccessor(0);
                    }
                }
            }
        }
    }
    return nullptr;
}


bool blockAccessAllocMemory(BasicBlock* block, CallInst* alloc) {
    if (block == nullptr or alloc == nullptr) {
        return false;
    }
    //auto allocatedValue = alloc->getValueID();
    for (const Instruction& inst : *block) {
        for (size_t i = 0; i < inst.getNumOperands(); ++i) {
            const Value* operandValue = inst.getOperand(i);

            if (operandValue == alloc) {
                return true;
            }
        }
    }
    return false;
}



bool needOptimization(BranchInst* weightedBranch, CallInst* alloc) {

}


// пройтись по всем бренчам, найти именно ту, которая является unlikely
// и еслив ней я буду использовать память и нигде больше, 
// то тогда Transformation pass

// New PM implementation
struct MemOpt : PassInfoMixin<MemOpt> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Function& func, FunctionAnalysisManager&) {
    CallInst* alloc = findHeapAllocation(func);
    BranchInst* weightedBranch = findWeightedBranch(func);



    if (blockAccessAllocMemory(weightedBranch->getSuccessor(0), alloc)) {
        errs() << "УРАААА\n";
    }



    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};

} // namespace

llvm::PassPluginLibraryInfo getHelloWorldPluginInfo() {
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

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize MemOpt when added to the pass pipeline on the
// command line, i.e. via '-passes=mem-opt'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getHelloWorldPluginInfo();
}
