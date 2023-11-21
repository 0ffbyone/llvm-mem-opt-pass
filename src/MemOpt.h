#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <exception>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/Casting.h>

#include <optional>
#include <utility>
#include <vector>

using namespace llvm;

namespace memopt {
CallInst* findHeapAllocation(Function &func);

struct BlockWeight {
  BlockWeight(){};
  BlockWeight(BasicBlock *block, int weight) 
      : block(block), weight(weight) {}

  BasicBlock *block{nullptr};
  int weight = INT_MAX;
};

// std::optional<std::pair<BlockWeight, BlockWeight>>
// branchWeights(const BranchInst& branchInst);

//BasicBlock* findUnlikelyBlock(const BranchInst &branchInst);
//BasicBlock* findUnlikelyBlock(SwitchInst& branchInst);

std::vector<std::variant<BranchInst*, SwitchInst*>>
findWeightedBranches(Function &func);

bool blockAccessAllocMemory(BasicBlock *block, CallInst *alloc);

//bool needOptimization(BranchInst* weightedBranch, CallInst *alloc);
//bool needOptimization(SwitchInst* weightedBranch, CallInst *alloc);

std::vector<BlockWeight> branchWeights(const BranchInst* branchInst);
std::vector<BlockWeight> branchWeights(SwitchInst* switchInst);


BasicBlock* findUnlikelyBlock(BranchInst& branchInst);
std::vector<BasicBlock*> findUnlikelyBlock(SwitchInst& switchInst);


bool needOptimization(BranchInst* weightedBranch, CallInst* alloc);
bool needOptimization(SwitchInst* weightedSwitch, CallInst* alloc);


//template <typename T = SwitchInst>
//std::vector<BlockWeight> branchWeights(const T &branchInst) {
//  std::vector<BlockWeight> weights{};
//
//  SwitchInstProfUpdateWrapper SIPUW = SwitchInstProfUpdateWrapper(branchInst);
//  MDNode *metaData;
//  errs() << "-1\n";
//  try {
//    metaData = branchInst.getMetadata(LLVMContext::MD_prof);
//    //auto wtf = branchInst.getPro
//
//  } catch (std::exception &e) {
//    errs() << e.what() << '\n';
//    return weights;
//  }
//  errs() << "0\n";
//  if (static_cast<bool>(metaData)) {
//    errs() << "1\n";
//    size_t numOfBranches = metaData->getNumOperands() - 1;
//    for (size_t i = 1; i < numOfBranches; ++i) {
//      const ConstantAsMetadata *weightMD =
//          dyn_cast<ConstantAsMetadata>(metaData->getOperand(i).get());
//      errs() << "2\n";
//      if (weightMD) {
//        errs() << "3\n";
//        ConstantInt *ciWeight = dyn_cast<ConstantInt>(weightMD->getValue());
//        if (ciWeight) {
//          errs() << "4\n";
//          int weight = ciWeight->getSExtValue();
//          weights.push_back(BlockWeight(branchInst.getSuccessor(i), weight));
//        }
//      }
//    }
//  }
//  errs() << "выход из branchWeights функции "
//         << "\n";
//  return weights;
//}

// Propagate existing explicit probabilities from either profile data or
// 'expect' intrinsic processing.
//bool BranchProbabilityInfo::calcMetadataWeights(BasicBlock *BB) {
//  TerminatorInst *TI = BB->getTerminator();
//  if (TI->getNumSuccessors() == 1)
//    return false;
//  if (!isa<BranchInst>(TI) && !isa<SwitchInst>(TI))
//    return false;
//
//  MDNode *WeightsNode = TI->getMetadata(LLVMContext::MD_prof);
//  if (!WeightsNode)
//    return false;
//
//  // Ensure there are weights for all of the successors. Note that the first
//  // operand to the metadata node is a name, not a weight.
//  if (WeightsNode->getNumOperands() != TI->getNumSuccessors() + 1)
//    return false;
//
//  // Build up the final weights that will be used in a temporary buffer, but
//  // don't add them until all weihts are present. Each weight value is clamped
//  // to [1, getMaxWeightFor(BB)].
//  uint32_t WeightLimit = getMaxWeightFor(BB);
//  SmallVector<uint32_t, 2> Weights;
//  Weights.reserve(TI->getNumSuccessors());
//  for (unsigned i = 1, e = WeightsNode->getNumOperands(); i != e; ++i) {
//    ConstantInt *Weight = dyn_cast<ConstantInt>(WeightsNode->getOperand(i));
//    if (!Weight)
//      return false;
//    Weights.push_back(
//        std::max<uint32_t>(1, Weight->getLimitedValue(WeightLimit)));
//  }
//  assert(Weights.size() == TI->getNumSuccessors() && "Checked above");
//  for (unsigned i = 0, e = TI->getNumSuccessors(); i != e; ++i)
//    setEdgeWeight(BB, i, Weights[i]);
//
//  return true;
//}

} // namespace memopt
