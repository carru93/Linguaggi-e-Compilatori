#include "LICM.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

PreservedAnalyses LICM::run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR, LPMUpdater &U)
{
    bool Transformed = false;

    return (Transformed ? PreservedAnalyses::none() : PreservedAnalyses::all());
}

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "MyLICM",
        .PluginVersion = "1.0",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB)
        {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, LoopPassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) -> bool
                {
                    if (Name == "MyLICM")
                    {
                        LPM.addPass(LoopInvariantCodeMotion());
                        return true;
                    }
                    return false;
                });
        }};
}