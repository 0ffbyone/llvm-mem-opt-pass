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
std::vector<const CallInst*> findHeapAllocation(Function& func) {
    std::vector<const CallInst*> allocations{};
    errs() << "In " << func.getName() << "\n";

    for (const BasicBlock& block : func) {
        for (const Instruction& inst : block) {
            auto* callInst = dyn_cast<CallInst>(&inst);
            if (callInst) {
                Function* calledFunc = callInst->getCalledFunction();

                errs() << "    "<< calledFunc->getName() << "()" << "\n";
                AttributeList attributes = calledFunc->getAttributes();
                //AllocFnKind alloc_kind = attributes.getAllocKind();

                //if (alloc_kind == AllocFnKind::Alloc) {
                //        errs() << "Alloc\n";
                //}

                //switch (alloc_kind) {
                //    case AllocFnKind::Alloc:
                //        errs() << "Alloc\n";
                //        break;
                //    case AllocFnKind::Free:
                //        errs() << "Free\n";
                //        break;
                //    case AllocFnKind::Zeroed:
                //        errs() << "Zeroed\n";
                //        break;
                //    case AllocFnKind::Realloc:
                //        errs() << "Realloc\n";
                //        break;
                //    case AllocFnKind::Aligned:
                //        errs() << "Aligned\n";
                //        break;
                //    case AllocFnKind::Uninitialized:
                //        errs() << "Uninitialized\n";
                //        break;
                //    case AllocFnKind::Unknown:
                //        errs() << "Unknown\n";
                //        break;
                //}


                auto isAllocFunc = attributes.hasFnAttr(Attribute::AllocSize);

                if (isAllocFunc) {
                    allocations.push_back(callInst);
                }
                //errs() << (isAllocFunc? "has alloc size": "no") << '\n';
            }
        }
    }
    return allocations;
}


void branchWeights(Function& func) {
    for (const BasicBlock& block : func) {
        for (const Instruction& inst : block) {
            const BranchInst* branchInst = dyn_cast<BranchInst>(&inst);
            if (branchInst) {
                //SmallVectorImpl<std::pair<uint32_t, MDNode*>> MDs;
                errs() << "in branch\n";
                MDNode* metaData = branchInst->getMetadata("branch_weights");
                if (metaData) {
                    errs() << "branch_weights\n";
                }
            }
        }
    }
}



// New PM implementation
struct MemOpt : PassInfoMixin<MemOpt> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Function& func, FunctionAnalysisManager&) {
    findHeapAllocation(func);
    branchWeights(func);
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
