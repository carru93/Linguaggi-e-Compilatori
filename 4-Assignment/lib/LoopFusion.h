#ifndef LLVM_TRANSFORMS_UTILS_LOOPFUSION_H
#define LLVM_TRANSFORMS_UTILS_LOOPFUSION_H

#include <llvm/IR/PassManager.h>
#include <llvm/IR/Constants.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/DependenceAnalysis.h>

namespace llvm
{
    struct LoopFusion : public PassInfoMixin<LoopFusion>
    {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
    };
}

#endif
