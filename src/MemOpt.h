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
#include <optional>
#include <utility>
#include <vector>

using namespace llvm;

namespace memopt {
CallInst* findHeapAllocation(Function &func);

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


BasicBlock* findUnlikelyBlock(BranchInst& branchInst);
std::vector<BasicBlock*> findUnlikelyBlock(SwitchInst& switchInst);


BasicBlock*
needOptimization(Function& func, BranchInst* weightedBranch, CallInst* alloc);

BasicBlock*
needOptimization(Function& func, SwitchInst* weightedSwitch, CallInst* alloc);


void moveAllocInsideWeightedBlock(CallInst* alloc, BasicBlock& weightedBlock);

} // namespace memopt
