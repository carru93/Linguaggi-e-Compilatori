#include "LoopFusion.h"
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>

using namespace llvm;

BasicBlock *getFirstBlockOfLoop(Loop *L)
{
    if (BasicBlock *PH = L->getLoopPreheader())
        return PH;

    return L->getHeader();
}

bool areLoopsAdjacent(Loop *L1, Loop *L2, LoopInfo &LI)
{
    if (!L1 || !L2)
        return false;

    bool G1 = L1->isGuarded();
    bool G2 = L2->isGuarded();

    if (G1 && G2)
    {
        BranchInst *GuardBr1 = L1->getLoopGuardBranch();
        if (!GuardBr1)
            return false;

        if (GuardBr1->getNumSuccessors() != 2)
            return false;

        BasicBlock *Succ0 = GuardBr1->getSuccessor(0);
        BasicBlock *Succ1 = GuardBr1->getSuccessor(1);

        bool Succ0InL1 = L1->contains(Succ0);
        bool Succ1InL1 = L1->contains(Succ1);

        if (Succ0InL1 == Succ1InL1)
            return false;

        BasicBlock *NonLoopSucc = Succ0InL1 ? Succ1 : Succ0;

        BranchInst *GuardBr2 = L2->getLoopGuardBranch();
        if (!GuardBr2)
            return false;

        BasicBlock *L2Entry = GuardBr2->getParent();

        return NonLoopSucc == L2Entry;
    }
    else if (!G1 && !G2)
    {
        BasicBlock *L1Exit = L1->getExitBlock();
        BasicBlock *L2Preheader = L2->getLoopPreheader();

        if (!L1Exit || !L2Preheader)
            return false;

        return L1Exit == L2Preheader;
    }

    return false;
}

bool haveSameTripCount(Loop *L1, Loop *L2, ScalarEvolution &SE)
{
    const SCEV *L1TripCount = SE.getBackedgeTakenCount(L1);
    const SCEV *L2TripCount = SE.getBackedgeTakenCount(L2);

    if (!L1TripCount || !L2TripCount || isa<SCEVCouldNotCompute>(L1TripCount) || isa<SCEVCouldNotCompute>(L2TripCount))
        return false;

    if (SE.isKnownPredicate(CmpInst::ICMP_EQ, L1TripCount, L2TripCount))
        return true;

    // Let give this a try...
    const SCEV *Diff = SE.getMinusSCEV(L1TripCount, L2TripCount);
    if (const SCEVConstant *DiffConst = dyn_cast<SCEVConstant>(Diff))
    {
        if (DiffConst->getValue()->isZero())
            return true;
    }

    return false;
}

bool controlFlowEquivalent(Loop *L1, Loop *L2, DominatorTree &DT, PostDominatorTree &PDT)
{
    BasicBlock *L1Start = getFirstBlockOfLoop(L1);
    BasicBlock *L2Start = getFirstBlockOfLoop(L2);
    BasicBlock *L1Exit = L1->getExitBlock();
    BasicBlock *L2Exit = L2->getExitBlock();

    if (!L1Start || !L2Start || !L1Exit || !L2Exit)
        return false;

    if (!DT.dominates(L1Start, L2Start))
        return false;

    if (!PDT.dominates(L2Exit, L1Exit))
        return false;

    return true;
}

bool noCrossIterationDependency(Loop *L1, Loop *L2, DependenceInfo &DI)
{
    for (BasicBlock *BB1 : L1->getBlocks())
    {
        for (Instruction &I1 : *BB1)
        {
            if (!I1.mayReadOrWriteMemory())
                continue;

            for (BasicBlock *BB2 : L2->getBlocks())
            {
                for (Instruction &I2 : *BB2)
                {

                    if (!I2.mayReadOrWriteMemory())
                        continue;

                    if (DI.depends(&I1, &I2, true))
                        return false;
                }
            }
        }
    }

    return true;
}

struct InductionInfo
{
    PHINode *Phi;
    const SCEV *Start; // SCEV del valore iniziale
    const SCEV *Step;  // SCEV del passo
};

bool extractInductionInfo(Loop *L, ScalarEvolution &SE, InductionInfo &OutInfo)
{
    BasicBlock *Header = L->getHeader();
    if (!Header)
        return false;

    PHINode *IndVar = L->getCanonicalInductionVariable();
    if (!IndVar)
        return false;

    OutInfo.Phi = IndVar;

    const SCEV *PhiSCEV = SE.getSCEV(IndVar);

    if (auto *AddRec = dyn_cast<SCEVAddRecExpr>(PhiSCEV))
    {
        OutInfo.Start = AddRec->getStart();
        OutInfo.Step = AddRec->getStepRecurrence(SE);

        if (!isa<SCEVConstant>(OutInfo.Step))
            return false;

        return true;
    }

    return false;
}

/**
 * for (int i = 0; i < 100; ++i) {}
 * for (int j = 100; j < 300; j += 2) {}
 *
 * j(i) = start2 + (stride2 / stride1) * (i - start1)
 *  => j(i) = 100 + (2 / 1) * (i - 0) = 100 + 2i
 */
Value *createIndVarMapping(IRBuilder<> &builder, Value *i, const SCEVConstant *start1,
                           const SCEVConstant *stride1,
                           const SCEVConstant *start2,
                           const SCEVConstant *stride2)
{
    const APInt &start1Val = start1->getAPInt();
    const APInt &stride1Val = stride1->getAPInt();
    const APInt &start2Val = start2->getAPInt();
    const APInt &stride2Val = stride2->getAPInt();

    uint64_t ratio = (stride2Val.getZExtValue() / stride1Val.getZExtValue());

    Value *Offset = builder.getInt64(start2Val.getZExtValue());
    Value *Start1 = builder.getInt64(start1Val.getZExtValue());
    Value *Ratio = builder.getInt64(ratio);

    Value *i64 = i;
    if (i->getType()->isIntegerTy())
    {
        unsigned bitw = cast<IntegerType>(i->getType())->getBitWidth();
        if (bitw < 64)
            i64 = builder.CreateZExt(i, builder.getInt64Ty(), "i.cast");
    }

    Value *diff = builder.CreateSub(i64, Start1, "diff");
    Value *scaled = builder.CreateMul(Ratio, diff, "scaled");
    Value *newJ = builder.CreateAdd(Offset, scaled, "newJ");

    return builder.CreateTrunc(newJ, builder.getInt32Ty(), "newJ.i32");
}

bool fuseLoops(Loop *L1, Loop *L2, LoopInfo &LI, DominatorTree &DT, ScalarEvolution &SE, DependenceInfo &DI)
{
    BasicBlock *L1Header = L1->getHeader();
    BasicBlock *L2Header = L2->getHeader();

    if (!L1Header || !L2Header)
        return false;

    InductionInfo Info1, Info2;
    if (!extractInductionInfo(L1, SE, Info1))
        return false;
    if (!extractInductionInfo(L2, SE, Info2))
        return false;

    auto *Start1C = dyn_cast<SCEVConstant>(Info1.Start);
    auto *Step1C = dyn_cast<SCEVConstant>(Info1.Step);
    auto *Start2C = dyn_cast<SCEVConstant>(Info2.Start);
    auto *Step2C = dyn_cast<SCEVConstant>(Info2.Step);

    if (!Start1C || !Step1C || !Start2C || !Step2C)
    {
        // non-constant start or step
        return false;
    }

    IRBuilder<> builder(&*L1Header->getFirstInsertionPt());
    Value *iVal = Info1.Phi;
    Value *jVal = createIndVarMapping(builder, iVal, Start1C, Step1C, Start2C, Step2C);

    PHINode *oldJVal = Info2.Phi;
    oldJVal->replaceAllUsesWith(jVal);

    BasicBlock *L1Exit = L1->getExitBlock();
    if (!L1Exit)
        return false;

    Function *F = L1Header->getParent();
    for (auto *BB : L2->getBlocks())
    {
        if (BB == L2->getLoopPreheader())
            continue;

        BasicBlock &LastBB = F->back();
        BB->moveAfter(&LastBB);
    }

    LI.erase(L2);

    if (auto L1Term = L1Exit->getTerminator())
    {
        L1Term->eraseFromParent();
        IRBuilder<> exitBuilder(L1Exit);
        exitBuilder.CreateBr(L2Header);
    }

    DT.recalculate(*L1Header->getParent());

    return true;
}

PreservedAnalyses LoopFusion::run(Function &F, FunctionAnalysisManager &AM)
{
    bool Transformed = false;

    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);

    std::vector<Loop *> Loops(LI.begin(), LI.end());
    std::reverse(Loops.begin(), Loops.end());

    for (size_t i = 0; i + 1 < Loops.size(); ++i)
    {
        Loop *L1 = Loops[i];
        Loop *L2 = Loops[i + 1];

        auto L1Name = L1->getHeader()->getName();
        auto L2Name = L2->getHeader()->getName();

        if (!L1->isLoopSimplifyForm() || !L2->isLoopSimplifyForm())
        {
            errs() << "One of the loop is note simplified\n";
            continue;
        }
        errs() << "Passed loop simplify check\n";

        if (!areLoopsAdjacent(L1, L2, LI))
        {
            errs() << "Loops are not adjacent\n";
            continue;
        }
        errs() << "Passed adjacent check\n";

        if (!haveSameTripCount(L1, L2, SE))
        {
            errs() << "Lopps do not have same trip count\n";
            continue;
        }
        errs() << "Passed trip count check\n";

        if (!controlFlowEquivalent(L1, L2, DT, PDT))
        {
            errs() << "Control flow is not equivalent\n";
            continue;
        }
        errs() << "Passed flow equivalent check\n";

        if (!noCrossIterationDependency(L1, L2, DI))
        {
            errs() << "There are dependencies between cross iterations\n";
            continue;
        }
        errs() << "Passed cross iteration check\n";

        if (fuseLoops(L1, L2, LI, DT, SE, DI))
        {
            Transformed = true;
            i++; // Skip fused loop
            errs() << "Fused loops: " << L1Name << " and " << L2Name << "\n";
        }
        else
        {
            errs() << "Something went wrong during loop fuse\n";
        }
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