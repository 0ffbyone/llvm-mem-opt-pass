#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <optional>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>
#include <cstdlib>

#include "MemOpt.h"

using namespace llvm;
namespace memopt {
CallInst* findHeapAllocation(Function& func) {
    for (BasicBlock& block : func) {
        for (Instruction& inst: block) {
            auto* callInst = dyn_cast<CallInst>(&inst);
            if (callInst) {
                Function* calledFunc = callInst->getCalledFunction();
                AttributeList attributes =  calledFunc->getAttributes();
                auto isAllocFunc = attributes.hasFnAttr(Attribute::AllocSize);
                if (isAllocFunc) {
                    return callInst;
                }
            }
        }
    }

    return nullptr;
}


std::vector<BlockWeight> branchWeights(const BranchInst* branchInst) {
    std::vector<BlockWeight> weights;
    weights.reserve(2);

    MDNode* metaData = branchInst->getMetadata(LLVMContext::MD_prof);
    if(branchInst->isConditional() and static_cast<bool>(metaData)) {
        for (size_t i = 1; i < metaData->getNumOperands() - 1; ++i) {
            const ConstantAsMetadata* weightMD = dyn_cast<ConstantAsMetadata>
                                            (metaData->getOperand(i).get());
            if (weightMD) {
                ConstantInt* ciWeight = dyn_cast<ConstantInt>(weightMD->getValue());
                if (ciWeight) {
                    weights.push_back(BlockWeight(branchInst->getSuccessor(i - 1),
                              ciWeight->getSExtValue()));
                }
            }
        }
    }

    return weights;
}


std::vector<BlockWeight> branchWeights(SwitchInst* switchInst) {
    std::vector<BlockWeight> weights{};
    SwitchInstProfUpdateWrapper profWrapper = SwitchInstProfUpdateWrapper(*switchInst);
    for (size_t i = 0; i < switchInst->getNumSuccessors(); ++i) {
        SwitchInstProfUpdateWrapper::CaseWeightOpt weightOpt =
            profWrapper.getSuccessorWeight(i);
        if (weightOpt) {
            weights.push_back(BlockWeight(switchInst->getSuccessor(i),
                        weightOpt.value()));
        }
    }

    return weights;
}


std::vector<std::variant<BranchInst*, SwitchInst*>>
findWeightedBranches(Function& func)
{
    std::vector<std::variant<BranchInst*, SwitchInst*>> weightedBranches;
    std::variant<BranchInst*, SwitchInst*> branchOrSwitch;
    for (BasicBlock& block: func) {
        for (Instruction& inst : block) {
            BranchInst* branchInst = dyn_cast<BranchInst>(&inst);
            SwitchInst* switchInst = dyn_cast<SwitchInst>(&inst);

            if (branchInst) {
                branchOrSwitch = branchInst;
            } else if (switchInst){
                branchOrSwitch = switchInst;
            }
            if (branchInst or switchInst) {
                if (std::visit([](auto&& arg)
                { return branchWeights(arg).size(); }, branchOrSwitch) != 0)
                {
                    weightedBranches.push_back(branchOrSwitch);
                }
            }
        }
    }
    return weightedBranches;
}


bool blockAccessAllocMemory(BasicBlock* block, CallInst* alloc) {
    if (block == nullptr or alloc == nullptr) {
        return false;
    }

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


BasicBlock* findUnlikelyBlock(BranchInst& branchInst) {
    std::vector<BlockWeight> weights = branchWeights(&branchInst);

    BlockWeight minWeightBlock;
    for (const BlockWeight& blockWeight : weights) {
        if (minWeightBlock.weight > blockWeight.weight) {
            minWeightBlock = blockWeight;
        }
    }
    return minWeightBlock.block;
}


std::vector<BasicBlock*> findUnlikelyBlock(SwitchInst& switchInst) {
    std::vector<BasicBlock*> unlikelyBlocks{};
    std::vector<BlockWeight> weights = branchWeights(&switchInst);

    uint32_t defaultWeight = weights[0].weight;
    for (const BlockWeight& blockWeight : weights) {
        if (blockWeight.weight < defaultWeight) {
            unlikelyBlocks.push_back(blockWeight.block);
        }
    }

    return unlikelyBlocks;
}


BasicBlock*
needOptimization(Function& func, BranchInst* weightedBranch, CallInst* alloc) {
    BasicBlock* blockForOptimization = nullptr;
    if (weightedBranch == nullptr or alloc == nullptr) {
        return nullptr;
    }
    BasicBlock* unlikelyBlock = findUnlikelyBlock(*weightedBranch);

    for (BasicBlock& currBlock : func) {
        bool accessingAllocMemoryInBlock = blockAccessAllocMemory(&currBlock, alloc);
        if (&currBlock == unlikelyBlock and
            accessingAllocMemoryInBlock) {
            blockForOptimization = &currBlock;
        } else if (&currBlock != unlikelyBlock and
                accessingAllocMemoryInBlock) {
            return nullptr;
        }
    }

    return blockForOptimization;
}


BasicBlock*
needOptimization(Function& func, SwitchInst* weightedSwitch, CallInst* alloc) {
    BasicBlock* blockForOptimization = nullptr;
    if (weightedSwitch == nullptr or alloc == nullptr) {
        return nullptr;
    }
    std::vector<BasicBlock*> unlikelyBlocks = findUnlikelyBlock(*weightedSwitch);

    bool inUse = false;
    size_t i = 0;
    for (BasicBlock& currBlock : func) {
        bool accessingAllocMemoryInBlock = blockAccessAllocMemory(&currBlock, alloc);
        bool currBlockIsUnlikely = (&currBlock == unlikelyBlocks[i]);

        if (currBlockIsUnlikely and not inUse) {
            if (accessingAllocMemoryInBlock) {
                inUse = true;
                blockForOptimization = &currBlock;
            }
            ++i;
        } else if (currBlockIsUnlikely and inUse and accessingAllocMemoryInBlock) {
            return nullptr;
        } else if (not currBlockIsUnlikely and accessingAllocMemoryInBlock) {
            return nullptr;
        }
    }

    return blockForOptimization;
}


void moveAllocInsideWeightedBlock(CallInst* alloc, BasicBlock& weightedBlock) {
    auto insertionPt = weightedBlock.getFirstInsertionPt();
    alloc->moveBefore(weightedBlock, insertionPt);
}
} // namespace memopt
