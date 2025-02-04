#ifndef LLVM_TRANSFORMS_UTILS_LOCALOPTS_H
#define LLVM_TRANSFORMS_UTILS_LOCALOPTS_H

#include <llvm/IR/PassManager.h>
#include <llvm/IR/Constants.h>

namespace llvm
{
    struct AlgebraicIdentity : public PassInfoMixin<AlgebraicIdentity>
    {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    };

    struct StrengthReduction : public PassInfoMixin<StrengthReduction>
    {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    };

    struct MultiInstOpt : public PassInfoMixin<MultiInstOpt>
    {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    };
}

#endif
