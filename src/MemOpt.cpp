#include "llvm/IR/LegacyPassManager.h"
#include <cstdint>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <algorithm>
#include <llvm/Support/raw_ostream.h>
#include <variant>
#include <vector>
#include <cstdlib>

#include "MemOpt.h"

using namespace llvm;
using std::vector;

namespace memopt {
vector<CallInst*> findHeapAllocations(Function& func) {
    vector<CallInst*> allocs;
    for (BasicBlock& block : func) {
        for (Instruction& inst: block) {
            auto* callInst = dyn_cast<CallInst>(&inst);
            if (not callInst) {
                continue;
            }
            Function* calledFunc = callInst->getCalledFunction();
            if (not calledFunc) {
                continue;
            }
            AttributeList attributes =  calledFunc->getAttributes();
            bool isAllocFunc = attributes.hasFnAttr(Attribute::AllocKind);
            if (not isAllocFunc) {
                continue;
            }
            Attribute allocFuncKind = attributes.getFnAttr(Attribute::AllocKind);
            AllocFnKind kind = allocFuncKind.getAllocKind();

            if (static_cast<uint64_t>(kind) & static_cast<uint64_t>(AllocFnKind::Alloc)) {
                allocs.push_back(callInst);
            }
        }
    }

    return allocs;
}


vector<BlockWeight> branchWeights(const BranchInst* branchInst) {
    vector<BlockWeight> weights;
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


vector<BlockWeight> branchWeights(SwitchInst* switchInst) {
    vector<BlockWeight> weights{};
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


vector<std::variant<BranchInst*, SwitchInst*>>
findWeightedBranches(Function& func)
{
    vector<std::variant<BranchInst*, SwitchInst*>> weightedBranches;
    std::variant<BranchInst*, SwitchInst*> branchOrSwitch;
    for (BasicBlock& block: func) {
        for (Instruction& inst : block) {
            BranchInst* branchInst = dyn_cast<BranchInst>(&inst);
            SwitchInst* switchInst = dyn_cast<SwitchInst>(&inst);

            if (branchInst) {
                branchOrSwitch = branchInst;
            } else if (switchInst) {
                branchOrSwitch = switchInst;
            }

            if (not (branchInst or switchInst)) {
                continue;
            }

            auto numberOfWeightedBranches = std::visit([&](auto&& arg) { return branchWeights(arg).size(); }, branchOrSwitch);
            if (numberOfWeightedBranches != 0) {
                weightedBranches.push_back(branchOrSwitch);
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


void moveAllocInsideWeightedBlock(CallInst* alloc, BasicBlock& weightedBlock) {
    auto insertionPt = weightedBlock.getFirstInsertionPt();
    alloc->moveBefore(weightedBlock, insertionPt);
}


[[deprecated]] vector<BasicBlock*> getDominators(BasicBlock* basicBlock) {
    vector<BasicBlock*> dominators;
    DominatorTree domTree(*basicBlock->getParent());
    DomTreeNode *node = domTree.getNode(basicBlock);
    if (!node) {
      return dominators;
    }

    node = node->getIDom();
    while (node && node->getBlock()) {
      dominators.push_back(node->getBlock());
      node = node->getIDom();
    }

    return dominators;
}


Instruction* instNotPhi(User* user) {
    auto* inst = dyn_cast<Instruction>(user);
    bool phiNode = isa<PHINode>(inst);
    if (not inst or phiNode) {
        return nullptr;
    }
    return inst;
}


[[deprecated]] bool _dominatedByUnlikelyBlock(vector<BasicBlock*>& unlikelyBlocks, Instruction* inst)
{
    BasicBlock* parent = inst->getParent();
    vector<BasicBlock*> dominators = getDominators(parent);
    dominators.push_back(parent);


    //errs() << "current BB\n"<< *(inst->getParent()) << '\n';
    //for (const auto& el : dominators) {
    //    errs() << *el << '\n';
    //}


    std::sort(dominators.begin(), dominators.end());
    std::sort(unlikelyBlocks.begin(), unlikelyBlocks.end());

    vector<BasicBlock*> unlikelyDominators;
    std::ranges::set_intersection(unlikelyBlocks, dominators,
            std::back_inserter(unlikelyDominators));

    //if (unlikelyDominators.size() > 0) {
    //    std::cout << "some unlikely dominator\n";
    //} else {
    //    std::cout << "no unlikely dominators\n";
    //}
    return unlikelyDominators.size() > 0? true: false;

}


bool dominatedByUnlikelyBlock(Function* func, std::set<BasicBlock*>& unlikelyBlocks, BasicBlock* blockToCheck) {
    DominatorTree domTree(*func);
    for (auto& block : unlikelyBlocks) {
        Instruction* firstInst = block->getFirstNonPHI();
        bool currDom = domTree.dominates(firstInst, blockToCheck);
        if ((blockToCheck == block) or currDom) {
            return true;
        }
    }
    return false;
}

BasicBlock* needOptimization(Function* func, std::set<BasicBlock*>& unlikelyBlocks, CallInst* alloc) {
    BasicBlock* blockForOptimization = nullptr;
    //if (weightedT == nullptr or alloc == nullptr) {
    //    return nullptr;
    //}

    //std::vector<BasicBlock*> unlikelyBlocks = findUnlikelyBlocks(weightedT);
    //if (unlikelyBlocks.size() == 0) {
    //    return nullptr;
    //}

    DominatorTree domTree(*func);
    //std::vector<BasicBlock*> usersOfAlloc;
    Instruction* inst = nullptr;
    for (const auto& user : alloc->users()) {
        auto* currentInst = dyn_cast<Instruction>(user);
        if (inst == nullptr) {
            inst = currentInst;
        }
        Instruction* dominatingInst = leastCommonAncestorInstruction(&domTree, currentInst, inst);
        inst = dominatingInst;
    }
    bool phiNode = isa<PHINode>(inst);
    if (not inst or phiNode) {
        return nullptr;
    }
    BasicBlock* parent = inst->getParent();
    bool isDominatedByUnlikelyBlock = dominatedByUnlikelyBlock(func, unlikelyBlocks, parent);
    if (isDominatedByUnlikelyBlock and (not blockForOptimization
                    or blockForOptimization == parent)) {
        blockForOptimization = parent;
    } else {
        return nullptr;
    }


    return blockForOptimization;
}


Instruction* leastCommonAncestorInstruction(DominatorTree* domTree, Instruction* firstInstruction, Instruction* secondInstruction) {
    BasicBlock* firstBasicBlock = firstInstruction->getParent();
    BasicBlock* secondBasicBlock = secondInstruction->getParent();
    BasicBlock* dominatingBasicBlock = domTree->findNearestCommonDominator(firstBasicBlock, secondBasicBlock);
    if (firstBasicBlock == dominatingBasicBlock) {
        return firstInstruction;
    }
    if (secondBasicBlock == dominatingBasicBlock) {
        return secondInstruction;
    }
    return dominatingBasicBlock->getTerminator();
}



} // namespace memopt
