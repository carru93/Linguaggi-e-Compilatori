#include "LICM.h"

#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;

SmallVector<BasicBlock *> getDFSOrder(Loop *L)
{
    SmallVector<BasicBlock *> DFSOrder;
    SmallVector<BasicBlock *> WorkList;
    SmallPtrSet<BasicBlock *, 10> Visited;

    BasicBlock *Header = L->getHeader();
    WorkList.push_back(Header);

    while (!WorkList.empty())
    {
        BasicBlock *BB = WorkList.pop_back_val();
        if (!Visited.insert(BB).second)
            continue;
        DFSOrder.push_back(BB);
        for (BasicBlock *Succ : successors(BB))
        {
            if (L->contains(Succ))
                WorkList.push_back(Succ);
        }
    }
    return DFSOrder;
}

bool isInstructionLoopInvariant(Instruction *I, Loop *L, DominatorTree &DT)
{
    if (I->isTerminator() || I->mayHaveSideEffects() || I->isVolatile())
        return false;
    for (Use &Op : I->operands())
    {
        Value *V = Op.get();
        if (isa<Constant>(V))
            continue;
        if (Instruction *OpInst = dyn_cast<Instruction>(V))
        {
            if (L->contains(OpInst))
                return false;
            continue;
        }
    }
    return true;
}

bool dominatesAllLoopExits(BasicBlock *BB, Loop *L, DominatorTree &DT)
{
    SmallVector<BasicBlock *> ExitBlocks;
    L->getExitBlocks(ExitBlocks);
    for (BasicBlock *ExitBB : ExitBlocks)
        if (!DT.dominates(BB, ExitBB))
            return false;
    return true;
}

bool isUniqueDefinition(Instruction *I, Loop *L)
{
    // SSA granted
    return true;
}

bool dominatesAllUses(Instruction *I, DominatorTree &DT, Loop *L)
{
    for (User *U : I->users())
    {
        if (Instruction *UserInst = dyn_cast<Instruction>(U))
        {
            if (L->contains(UserInst) && !DT.dominates(I->getParent(), UserInst->getParent()))
                return false;
        }
    }
    return true;
}

bool dependenciesHoisted(Instruction *I, Loop *L, BasicBlock *Preheader)
{
    for (Use &Op : I->operands())
    {
        if (Instruction *OpInst = dyn_cast<Instruction>(Op.get()))
        {
            if (L->contains(OpInst) && OpInst->getParent() != Preheader)
                return false;
        }
    }
    return true;
}

bool isDeadAtLoopExit(Instruction *I, Loop *L)
{
    for (User *U : I->users())
    {
        if (Instruction *UserInst = dyn_cast<Instruction>(U))
            if (!L->contains(UserInst))
                return false;
    }
    return true;
}

PreservedAnalyses LICM::run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR, LPMUpdater &U)
{
    bool Transformed = false;
    DominatorTree &DT = AR.DT;

    BasicBlock *Preheader = L.getLoopPreheader();
    if (!Preheader)
    {
        errs() << "Loop has no preheader; skipping LICM\n";
        return PreservedAnalyses::all();
    }

    SmallVector<BasicBlock *> DFSOrder = getDFSOrder(&L);
    SmallVector<Instruction *> HoistCandidates;

    for (BasicBlock *BB : DFSOrder)
    {
        for (Instruction &I : *BB)
        {
            // outs() << "Checking instruction: " << I << ":" << "invariant:" << isInstructionLoopInvariant(&I, &L, DT) << " dominatesAllLoopExits:" << dominatesAllLoopExits(BB, &L, DT) << " isUniqueDefinition:" << isUniqueDefinition(&I, &L) << " dominatesAllUses:" << dominatesAllUses(&I, DT, &L) << "\n";
            if (!isInstructionLoopInvariant(&I, &L, DT))
                continue;

            if (!dominatesAllLoopExits(BB, &L, DT) && !isDeadAtLoopExit(&I, &L))
                continue;

            if (!isUniqueDefinition(&I, &L))
                continue;

            if (!dominatesAllUses(&I, DT, &L))
                continue;

            HoistCandidates.push_back(&I);
        }
    }

    for (Instruction *I : HoistCandidates)
    {
        if (!dependenciesHoisted(I, &L, Preheader))
            continue;

        I->moveBefore(Preheader->getTerminator());
        Transformed = true;
        errs() << "Hoisted instruction: " << *I << "\n";
    }

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
                [](StringRef Name, LoopPassManager &LPM, ArrayRef<PassBuilder::PipelineElement>) -> bool
                {
                    if (Name == "MyLICM")
                    {
                        LPM.addPass(LICM());
                        return true;
                    }
                    return false;
                });
        }};
}