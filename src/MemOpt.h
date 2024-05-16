#pragma once

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <algorithm>
#include <exception>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#include <iostream>
#include <variant>

using namespace llvm;

namespace memopt {
std::vector<CallInst*> findHeapAllocations(Function &func);

struct BlockWeight {
  BlockWeight(){};
  BlockWeight(BasicBlock *block, uint32_t weight)
      : block(block), weight(weight) {}

  BasicBlock *block{nullptr};
  uint32_t weight = UINT32_MAX;
};

std::vector<std::variant<BranchInst*, SwitchInst*>>
findWeightedBranches(Function &func);

bool blockAccessAllocMemory(BasicBlock *block, CallInst *alloc);


std::vector<BlockWeight> branchWeights(const BranchInst* branchInst);
std::vector<BlockWeight> branchWeights(SwitchInst* switchInst);

void moveAllocInsideWeightedBlock(CallInst* alloc, BasicBlock& weightedBlock);

Instruction* instNotPhi(User* user);

std::vector<BasicBlock*> getDominators(BasicBlock* basicBlock);

BasicBlock* lowestCommonAncestor(DominatorTree* dominatorTree, const std::vector<BasicBlock*>& unlikelyBlocks);

bool _dominatedByUnlikelyBlock(std::vector<BasicBlock*>& unlikelyBlocks, Instruction* inst);
bool dominatedByUnlikelyBlock(Function* func, std::set<BasicBlock*>& unlikelyBlocks, BasicBlock* blockToCheck);

Instruction* leastCommonAncestorInstruction(DominatorTree* domTree, Instruction* firstInstruction, Instruction* secondInstruction);


template<typename T>
std::set<BasicBlock*> findUnlikelyBlocks(T* TInst) {
    std::set<BasicBlock*> unlikelyBlocks{};
    std::vector<BlockWeight> weights;
    weights = branchWeights(TInst);
    if constexpr (std::is_same<T, BranchInst>::value) {
        BlockWeight minWeightBlock;
        for (const BlockWeight& blockWeight : weights) {
            if (minWeightBlock.weight > blockWeight.weight) {
                minWeightBlock = blockWeight;
            }
        }

        unlikelyBlocks.insert(minWeightBlock.block);
    } else {
        uint32_t defaultWeight = weights[0].weight;
        for (const BlockWeight& blockWeight : weights) {
            if (blockWeight.weight < defaultWeight) {
                unlikelyBlocks.insert(blockWeight.block);
            }
        }
    }

    return unlikelyBlocks;
}

BasicBlock* needOptimization(Function* func, std::set<BasicBlock*>& unlikelyBlocks, CallInst* alloc);

//template<typename T>
//BasicBlock* needOptimization(Function* func, T* weightedT, CallInst* alloc) {
//    BasicBlock* blockForOptimization = nullptr;
//    if (weightedT == nullptr or alloc == nullptr) {
//        return nullptr;
//    }
//
//    //std::vector<BasicBlock*> unlikelyBlocks = findUnlikelyBlocks(weightedT);
//    //if (unlikelyBlocks.size() == 0) {
//    //    return nullptr;
//    //}
//
//    for (const auto& user : alloc->users()) {
//        auto* inst = instNotPhi(user);
//        if (not inst) {
//            continue;
//        }
//
//        BasicBlock* parent = inst->getParent();
//        //std::cout << "before domination\n";
//        //bool isDominatedByUnlikelyBlock = dominatedByUnlikelyBlock(unlikelyBlocks, inst);
//        bool isDominatedByUnlikelyBlock = dominatedByUnlikelyBlock(func, unlikelyBlocks, inst);
//        if (isDominatedByUnlikelyBlock and (not blockForOptimization
//                        or blockForOptimization == parent)) {
//            blockForOptimization = parent;
//        } else {
//            return nullptr;
//        }
//    }
//
//    return blockForOptimization;
//}

} // namespace memopt
