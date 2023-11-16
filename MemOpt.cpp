#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <optional>
#include <utility>
#include <vector>

using namespace llvm;
namespace {
CallInst* findHeapAllocation(Function& func) {

    //errs() << "In " << func.getName() << '\n';

    for (BasicBlock& block : func) {
        for (Instruction& inst: block) {
            auto* callInst = dyn_cast<CallInst>(&inst);
            // Только CallInst
            if (callInst) {
                Function* calledFunc = callInst->getCalledFunction();
                AttributeList attributes =  calledFunc->getAttributes();
                auto isAllocFunc = attributes.hasFnAttr(Attribute::AllocSize);

                //errs() << calledFunc->getName() << '\n';

                if (isAllocFunc) {
                    return callInst;
                }
            }
        }
    }
    return nullptr;
}



struct BlockWeight {
    BlockWeight(BasicBlock* block, int weight) 
        :block(block), weight(weight) {}
    BasicBlock* block;
    int weight;
};

std::optional<std::pair<BlockWeight, BlockWeight>>
branchWeights(const BranchInst& branchInst) {
    int firstWeight;
    int secondWeight;
    MDNode* metaData = branchInst.getMetadata(LLVMContext::MD_prof);
    if(branchInst.isConditional() and static_cast<bool>(metaData)) {
        const ConstantAsMetadata* firstWeightMD = dyn_cast<ConstantAsMetadata>
                                        (metaData->getOperand(1).get());
        const ConstantAsMetadata* secondWeightMD = dyn_cast<ConstantAsMetadata>
                                        (metaData->getOperand(2));

        if (firstWeightMD and secondWeightMD) {
            Constant* firstWeightConstant = firstWeightMD->getValue();
            Constant* secondWeightConstant = secondWeightMD->getValue();

            ConstantInt * ciFirst = dyn_cast<ConstantInt>(firstWeightConstant);
            ConstantInt * ciSecond = dyn_cast<ConstantInt>(secondWeightConstant);

            if (ciFirst and ciSecond) {
                firstWeight = ciFirst->getSExtValue();
                secondWeight = ciSecond->getSExtValue();
                //return std::make_pair(firstWeight, secondWeight);
                return std::make_pair(
                        BlockWeight(branchInst.getSuccessor(0), firstWeight),
                        BlockWeight(branchInst.getSuccessor(1), secondWeight));
            }
        }
    }
    return std::nullopt;
}

BasicBlock* findUnlikelyBlock(const BranchInst& branchInst) {
    std::pair<BlockWeight, BlockWeight> weights = branchWeights(branchInst).value();
    return weights.first.weight < weights.second.weight ?
        weights.first.block : weights.second.block;
}


BranchInst* findWeightedBranch(Function& func) {
    for (BasicBlock& block: func) {
        for (Instruction& inst : block) {
            BranchInst* branchInst = dyn_cast<BranchInst>(&inst);
            // Только br инструкции
            if (branchInst) {
                // только с весом
                if (branchWeights(*branchInst)) {
                    return branchInst;
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
    if (weightedBranch == nullptr or alloc == nullptr) {
        return false;
    }
    BasicBlock* unlikelyBlock = findUnlikelyBlock(*weightedBranch);

    if (blockAccessAllocMemory(unlikelyBlock, alloc)) {
        return true;
    }

    return false;
}


// пройтись по всем бренчам, найти именно ту, которая является unlikely
// если в ней используется память, то тогда Transformation pass
struct MemOpt : PassInfoMixin<MemOpt> {
  PreservedAnalyses run(Function& func, FunctionAnalysisManager&) {
    CallInst* alloc = findHeapAllocation(func);
    BranchInst* weightedBranch = findWeightedBranch(func);

    if (needOptimization(weightedBranch, alloc)) {
        errs() << "можно оптимизировать\n";
    } else {
        errs() << "нельзя оптимизировать\n";
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
