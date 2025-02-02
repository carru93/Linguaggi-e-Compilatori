#include "LocalOpts.h"
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "LocalOpts",
        .PluginVersion = "1.0",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB)
        {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) -> bool
                {
                    if (Name == "ai")
                    {
                        MPM.addPass(AlgebraicIdentity());
                        return true;
                    }
                    if (Name == "strength") {
                        MPM.addPass(StrengthReduction());
                        return true;
                    }
                    return false;
                });
        }};
}