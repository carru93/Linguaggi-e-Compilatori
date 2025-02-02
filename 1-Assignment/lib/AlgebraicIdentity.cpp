#include "LocalOpts.h"
#include "llvm/IR/InstrTypes.h"

using namespace llvm;

static bool runOnFunction(Function &F);
bool algIdentity(BasicBlock &B);

PreservedAnalyses AlgebraicIdentity::run(Module &M, ModuleAnalysisManager &)
{
    bool Transformed = false;
    for (auto &F : M)
        if (runOnFunction(F))
            Transformed = true;

    return (Transformed ? PreservedAnalyses::none() : PreservedAnalyses::all());
}

static bool runOnFunction(Function &F)
{
    bool Transformed = false;

    for (auto &BB : F)
        if (algIdentity(BB))
            Transformed = true;

    return Transformed;
}

bool algIdentity(BasicBlock &BB)
{
    bool Transformed = false;
    auto InstIt = BB.begin();
    while (InstIt != BB.end())
    {
        Instruction &Inst = *InstIt;
        auto NextInstIt = std::next(InstIt);

        // Check if the instruction is a binary operation
        if (Inst.getNumOperands() == 2)
        {
            switch (Inst.getOpcode())
            {
            /* ADD -> x+0=x || 0+x=x */
            case Instruction::Add:
            {
                if (ConstantInt *C0 = dyn_cast<ConstantInt>(Inst.getOperand(0)))
                {
                    if (C0->isZero())
                    {
                        Inst.replaceAllUsesWith(Inst.getOperand(1));
                        Inst.eraseFromParent();
                        Transformed = true;
                        InstIt = NextInstIt;
                        continue;
                    }
                }
                else if (ConstantInt *C1 = dyn_cast<ConstantInt>(Inst.getOperand(1)))
                {
                    if (C1->isZero())
                    {
                        Inst.replaceAllUsesWith(Inst.getOperand(0));
                        Inst.eraseFromParent();
                        Transformed = true;
                        InstIt = NextInstIt;
                        continue;
                    }
                }
                break;
            }
            /* MUL -> x*1=x || 1*x=x */
            case Instruction::Mul:
            {
                if (ConstantInt *C0 = dyn_cast<ConstantInt>(Inst.getOperand(0)))
                {
                    if (C0->isOne())
                    {
                        Inst.replaceAllUsesWith(Inst.getOperand(1));
                        Inst.eraseFromParent();
                        Transformed = true;
                        InstIt = NextInstIt;
                        continue;
                    }
                }
                else if (ConstantInt *C1 = dyn_cast<ConstantInt>(Inst.getOperand(1)))
                {
                    if (C1->isOne())
                    {
                        Inst.replaceAllUsesWith(Inst.getOperand(0));
                        Inst.eraseFromParent();
                        Transformed = true;
                        InstIt = NextInstIt;
                        continue;
                    }
                }
                break;
            }
            /* SUB -> x-0=x */
            case Instruction::Sub:
            {
                if (ConstantInt *C1 = dyn_cast<ConstantInt>(Inst.getOperand(1)))
                {
                    if (C1->isZero())
                    {
                        Inst.replaceAllUsesWith(Inst.getOperand(0));
                        Inst.eraseFromParent();
                        Transformed = true;
                        InstIt = NextInstIt;
                        continue;
                    }
                }
                break;
            }
            /* DIV -> x/1=x */
            case Instruction::SDiv:
            {
                if (ConstantInt *C1 = dyn_cast<ConstantInt>(Inst.getOperand(1)))
                {
                    if (C1->isOne())
                    {
                        Inst.replaceAllUsesWith(Inst.getOperand(0));
                        Inst.eraseFromParent();
                        Transformed = true;
                        InstIt = NextInstIt;
                        continue;
                    }
                }
                break;
            }
            default:
                break;
            }
        }
        InstIt = NextInstIt;
    }
    return Transformed;
}
