#include <optional>
#include <unordered_map>

#include "LocalOpts.h"
#include "llvm/IR/InstrTypes.h"

using namespace llvm;

static bool runOnFunction(Function &F);
bool multiInstOpt(BasicBlock &BB);
static std::optional<std::pair<Value *, ConstantInt *>> getOptOperands(const Instruction &Inst);

PreservedAnalyses MultiInstOpt::run(Module &M, ModuleAnalysisManager &)
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
        if (multiInstOpt(BB))
            Transformed = true;

    return Transformed;
}

bool multiInstOpt(BasicBlock &BB)
{
    bool Transformed = false;
    auto InstIt = BB.begin();
    while (InstIt != BB.end())
    {
        Instruction &Inst = *InstIt;
        auto NextInstIt = std::next(InstIt);
        if (Inst.getNumOperands() == 2)
        {
            auto OptOperands = getOptOperands(Inst);
            if (!OptOperands)
            {
                InstIt = NextInstIt;
                continue;
            }

            Value *nonConstOp = OptOperands->first;
            ConstantInt *constOp = OptOperands->second;

            Instruction *prevInst = dyn_cast<Instruction>(nonConstOp);
            if (!prevInst)
            {
                InstIt = NextInstIt;
                continue;
            }

            auto prevOptOperands = getOptOperands(*prevInst);
            if (!prevOptOperands)
            {
                InstIt = NextInstIt;
                continue;
            }

            Value *prevNonConstOp = prevOptOperands->first;
            ConstantInt *prevConstOp = prevOptOperands->second;

            outs() << constOp->getValue() << " " << prevConstOp->getValue();

            if (constOp->getValue() != prevConstOp->getValue())
            {
                InstIt = NextInstIt;
                continue;
            }

            bool inverse = false;
            if (Inst.getOpcode() == Instruction::Sub && prevInst->getOpcode() == Instruction::Add)
                inverse = true;
            else if (Inst.getOpcode() == Instruction::Add && prevInst->getOpcode() == Instruction::Sub)
                inverse = true;

            if (!inverse)
            {
                InstIt = NextInstIt;
                continue;
            }

            // Debug: stampiamo informazioni sul pattern rilevato.
            errs() << "Multi-instruction optimization triggered:\n";
            errs() << "  Current instruction: " << Inst << "\n";
            errs() << "  Replacing with value from previous instruction: " << *prevNonConstOp << "\n";

            Inst.replaceAllUsesWith(prevNonConstOp);
            Transformed = true;
        }
        InstIt = NextInstIt;
    }

    return Transformed;
}

static std::optional<std::pair<Value *, ConstantInt *>> getOptOperands(const Instruction &Inst)
{
    if (Inst.getOpcode() != Instruction::Add && Inst.getOpcode() != Instruction::Sub)
        return std::nullopt;

    Value *Op1 = Inst.getOperand(0);
    Value *Op2 = Inst.getOperand(1);
    ConstantInt *CI = nullptr;

    // If the first operand is a constant, swap the operands
    if (Inst.getOpcode() == Instruction::Add && (CI = dyn_cast<ConstantInt>(Op1)))
    {
        std::swap(Op1, Op2);
    }

    CI = dyn_cast<ConstantInt>(Op2);
    if (!CI)
        return std::nullopt;

    return std::make_optional(std::make_pair(Op1, CI));
}