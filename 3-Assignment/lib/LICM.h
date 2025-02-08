#ifndef LLVM_TRANSFORMS_UTILS_MYLICM_H
#define LLVM_TRANSFORMS_UTILS_MYLICM_H

#include <llvm/IR/PassManager.h>
#include <llvm/IR/Constants.h>

namespace llvm
{
    struct LICM : public PassInfoMixin<LICM>
    {
        PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR, LPMUpdater &U);
    };
}

#endif
