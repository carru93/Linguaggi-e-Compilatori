#include "LocalOpts.h"
#include "llvm/IR/InstrTypes.h"

using namespace llvm;

int nearestPowerOfTwo(uint64_t value)
{
    if (value == 0)
        return 0;
    return 1 << (63 - __builtin_clzll(value));
}

int nearestLogBase2(uint64_t value)
{
    return 63 - __builtin_clzll(value);
}

static void insertInstruction(Instruction::BinaryOps type, Value *op, int power, Instruction &Inst, int rest = 0)
{
    Instruction *NewInst = BinaryOperator::Create(type, op, ConstantInt::get(Inst.getContext(), APInt(32, power)));
    NewInst->insertAfter(&Inst);
    Inst.replaceAllUsesWith(NewInst);

    if (rest == -1)
    {
        Instruction *Sub = BinaryOperator::Create(Instruction::Sub, NewInst, op);
        Sub->insertAfter(NewInst);
    }
    else if (rest == 1)
    {
        Instruction *Add = BinaryOperator::Create(Instruction::Add, NewInst, op);
        Add->insertAfter(NewInst);
    }
}

static bool runOnFunction(Function &F);
bool strReduction(BasicBlock &B);

PreservedAnalyses StrengthReduction::run(Module &M, ModuleAnalysisManager &)
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
        if (strReduction(BB))
            Transformed = true;

    return Transformed;
}

bool strReduction(BasicBlock &BB)
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
            case Instruction::Mul:
            {
                ConstantInt *C = nullptr;
                Value *otherValue = nullptr;
                if (ConstantInt *Const0 = dyn_cast<ConstantInt>(Inst.getOperand(0)))
                {
                    C = Const0;
                    otherValue = Inst.getOperand(1);
                }
                else if (ConstantInt *Const1 = dyn_cast<ConstantInt>(Inst.getOperand(1)))
                {
                    C = Const1;
                    otherValue = Inst.getOperand(0);
                }
                if (C)
                {
                    uint64_t value = C->getZExtValue();
                    int exp = nearestLogBase2(value);
                    int low = 1 << exp;
                    int high = low << 1;
                    int chosenPower;
                    if ((value - low) > (high - value))
                        chosenPower = high;
                    else
                        chosenPower = low;

                    int rest = int(value) - chosenPower;
                    if (abs(rest) <= 1)
                    {
                        int shiftExp = (chosenPower == low) ? exp : exp + 1;
                        insertInstruction(Instruction::Shl, otherValue, shiftExp, Inst, rest);
                        Transformed = true;
                        Inst.eraseFromParent();
                        InstIt = NextInstIt;
                        continue;
                    }
                }
                break;
            }
            case Instruction::SDiv:
            {
                if (ConstantInt *C = dyn_cast<ConstantInt>(Inst.getOperand(1)))
                {
                    uint64_t value = C->getZExtValue();
                    int exp = nearestLogBase2(value);
                    int low = 1 << exp;
                    int high = low << 1;
                    int chosenPower;
                    if ((value - low) > (high - value))
                        chosenPower = high;
                    else
                        chosenPower = low;
                    int rest = int(value) - chosenPower;
                    if (abs(rest) <= 1)
                    {
                        int shiftExp = (chosenPower == low) ? exp : exp + 1;
                        insertInstruction(Instruction::AShr, Inst.getOperand(0), shiftExp, Inst, rest);
                        Transformed = true;
                        Inst.eraseFromParent();
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