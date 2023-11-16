std::vector<CallInst*> findHeapAllocation(Function& func) {
    std::vector<CallInst*> allocations{};
    errs() << "In " << func.getName() << "\n";

    for (BasicBlock& block : func) {
        for (Instruction& inst : block) {
            auto* callInst = dyn_cast<CallInst>(&inst);
            if (callInst) {
                Function* calledFunc = callInst->getCalledFunction();

                errs() << "    "<< calledFunc->getName() << "()" << "\n";
                AttributeList attributes = calledFunc->getAttributes();
                auto isAllocFunc = attributes.hasFnAttr(Attribute::AllocSize);

                if (isAllocFunc) {
                    allocations.push_back(callInst);
                    errs() << "allocation\n";
                }
            } }
    }
    return allocations;
}



std::vector<BasicBlock*> branchWeights(Function& func) {
    std::vector<BasicBlock*> unlikelyBlocks;
    for (const BasicBlock& block : func) {
        for (const Instruction& inst : block) {
            const BranchInst* branchInst = dyn_cast<BranchInst>(&inst);
            if (branchInst) {
                errs() << "in branch\n";
                MDNode* metaData = branchInst->getMetadata("prof");
                if (branchInst->isConditional() and metaData) {
                    uint64_t branchVal = 0;
                    branchInst->extractProfTotalWeight(branchVal);
                    errs() << "branch_weights " << branchVal << "\n";
                    if (branchVal >= 2000) {
                        unlikelyBlocks.push_back(branchInst->getSuccessor(0));
                    }
                }
            }
        }
    }
    return unlikelyBlocks;
}




bool instAccessToVar(const Instruction* inst, const Value* operand) {
    for (const Use& use : inst->operands()) {
        if (use->getValueID() == operand->getValueID()) {
            return true;
        }
    }
    return false;
}


void foo(const std::vector<CallInst*>& allocs, BasicBlock* block) {
    for (Instruction& inst : *block) {

    }
}



  PreservedAnalyses run(Function& func, FunctionAnalysisManager&) {
    auto allocs = findHeapAllocation(func);
    auto unlikelyBlocks = branchWeights(func);

    for (BasicBlock* block : unlikelyBlocks) {
        for (auto& inst  : *block) {
            for (CallInst* callInst : allocs) {
                for (auto& operand : callInst->getOperandList()) {
                
                }
            }
        }
    }



    return PreservedAnalyses::all();
  }
