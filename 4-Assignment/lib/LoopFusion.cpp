#include "LoopFusion.h"
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>

using namespace llvm;

PreservedAnalyses LoopFusion::run(Function &F, FunctionAnalysisManager &AM)
{
    bool Transformed = false;

    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);

    for (auto *L : LI)
    {
    }

    return (Transformed ? PreservedAnalyses::none() : PreservedAnalyses::all());
}

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "LoopFusion",
        .PluginVersion = "1.0",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB)
        {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) -> bool
                {
                    if (Name == "LoopFusion")
                    {
                        FPM.addPass(LoopFusion());
                        return true;
                    }
                    return false;
                });
        }};
}