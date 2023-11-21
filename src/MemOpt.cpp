#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <exception>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "MemOpt.h"

using namespace llvm;
namespace memopt {
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

std::vector<BlockWeight> branchWeights(const BranchInst* branchInst) {
    std::vector<BlockWeight> weights;
    weights.reserve(2);
    MDNode* metaData = branchInst->getMetadata(LLVMContext::MD_prof);
    if(branchInst->isConditional() and static_cast<bool>(metaData)) {
        for (size_t i = 1; i < metaData->getNumOperands() - 1; ++i) {
            errs() << "BranchInst metadata getNumOperands - 1 == " << metaData->getNumOperands() - 1 << '\n';
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
    //errs() << "1\n";
    SwitchInstProfUpdateWrapper profWrapper = SwitchInstProfUpdateWrapper(*switchInst);
    //errs() << "2\n";
    for (size_t i = 0; i < switchInst->getNumSuccessors(); ++i) {
        errs() << "SwitchInst metadata getNumSuccessors == " << switchInst->getNumSuccessors() << '\n';
        //errs() << "num successors "<< switchInst->getNumSuccessors() << '\n';
        SwitchInstProfUpdateWrapper::CaseWeightOpt weightOpt =
            profWrapper.getSuccessorWeight(i);
        //errs() << "3\n";
        if (weightOpt) {
            //errs() << "4\n";
            weights.push_back(BlockWeight(switchInst->getSuccessor(i),
                        weightOpt.value()));
            //errs() << "5\n";
        }
    }

    return weights;
}



//BasicBlock* findUnlikelyBlock(const BranchInst& branchInst) {
//    //std::pair<BlockWeight, BlockWeight> weights = branchWeights(branchInst).value();
//    std::vector<BlockWeight> weights = branchWeights(&branchInst);
//    //return weights.first.weight < weights.second.weight ?
//    //    weights.first.block : weights.second.block;
//
//    BlockWeight minWeight;
//    for (const auto& blockWeight : weights) {
//        if (minWeight.weight > blockWeight.weight) {
//            minWeight = blockWeight;
//        }
//    }
//    return minWeight.block;
//}
//
//
//BasicBlock* findUnlikelyBlock(SwitchInst& switchInst) {
//    //std::pair<BlockWeight, BlockWeight> weights = branchWeights(branchInst).value();
//    std::vector<BlockWeight> weights = branchWeights(&switchInst);
//    //return weights.first.weight < weights.second.weight ?
//    //    weights.first.block : weights.second.block;
//
//    BlockWeight minWeight;
//    for (const auto& blockWeight : weights) {
//        if (minWeight.weight > blockWeight.weight) {
//            minWeight = blockWeight;
//        }
//    }
//    return minWeight.block;
//}


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
            // Только br и switch инструкции
            if (branchInst or switchInst) {
                //errs() << "некоторая бренча\n";
                if (std::visit([](auto&& arg)
                { return branchWeights(arg).size(); }, branchOrSwitch) != 0)
                {
                    //errs() << "нашлась вейтед\n";
                    //weightedBranches.push_back(switchInst);
                    weightedBranches.push_back(branchOrSwitch);
                }
            }
        }
    }
    //errs() << "выход из findWeightedBranches\n";
    return weightedBranches;
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

BasicBlock* findUnlikelyBlock(BranchInst& branchInst) {
    //std::pair<BlockWeight, BlockWeight> weights = branchWeights(branchInst).value();
    std::vector<BlockWeight> weights = branchWeights(&branchInst);
    //return weights.first.weight < weights.second.weight ?
    //    weights.first.block : weights.second.block;


    BlockWeight minWeightBlock;
    for (const BlockWeight& blockWeight : weights) {
        if (minWeightBlock.weight > blockWeight.weight) {
            //errs() << "min weight " << minWeightBlock.weight <<
            //    ", curr block weight " << blockWeight.weight << '\n';
            minWeightBlock = blockWeight;
        }
    }
    return minWeightBlock.block;
}


std::vector<BasicBlock*> findUnlikelyBlock(SwitchInst& switchInst) {
    std::vector<BasicBlock*> unlikelyBlocks{};
    std::vector<BlockWeight> weights = branchWeights(&switchInst);

    int defaultWeight = weights[0].weight;
    for (const BlockWeight& blockWeight : weights) {
        if (blockWeight.weight < defaultWeight) {
            unlikelyBlocks.push_back(blockWeight.block);
        }
    }

    return unlikelyBlocks;
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


bool needOptimization(SwitchInst* weightedSwitch, CallInst* alloc) {
    if (weightedSwitch == nullptr or alloc == nullptr) {
        return false;
    }
    std::vector<BasicBlock*> unlikelyBlocks = findUnlikelyBlock(*weightedSwitch);

    bool inUse = false;
    for (BasicBlock* block : unlikelyBlocks) {
        bool accessingAllocMemoryInBlock = blockAccessAllocMemory(block, alloc);
        if (not inUse and accessingAllocMemoryInBlock) {
            inUse = true;
        } else if (inUse and accessingAllocMemoryInBlock) {
            return false;
        }
    }
    // TODO: проверить, что не аллоцированная память не используется
    // нигде кроме данного unlikely блока


    return inUse;
}



//bool needOptimization(BranchInst* weightedBranch, CallInst* alloc) {
//    if (weightedBranch == nullptr or alloc == nullptr) {
//        return false;
//    }
//    BasicBlock* unlikelyBlock = findUnlikelyBlock(*weightedBranch);
//
//    if (blockAccessAllocMemory(unlikelyBlock, alloc)) {
//        return true;
//    }
//
//    return false;
//}
//
//bool needOptimization(SwitchInst* weightedBranch, CallInst* alloc) {
//    if (weightedBranch == nullptr or alloc == nullptr) {
//        return false;
//    }
//    BasicBlock* unlikelyBlock = findUnlikelyBlock(*weightedBranch);
//
//    if (blockAccessAllocMemory(unlikelyBlock, alloc)) {
//        return true;
//    }
//
//    return false;
//}


} // namespace memopt



