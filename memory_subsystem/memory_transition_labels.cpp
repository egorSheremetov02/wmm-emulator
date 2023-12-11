#include "memory_transition_labels.h"

#include <optional>

struct InstructionToLabelConverter {
    const ThreadLocalStorage& registers;
    MemoryTransitionLabel result;
    void operator()(const CasInstruction& instruction) {
        result = RmwLabel{
                instruction.mode,
                registers.GetRegisterValue(instruction.addr),
                [
                        expected = registers.GetRegisterValue(instruction.expected),
                        desired = registers.GetRegisterValue(instruction.desired)
                ] (uint64_t &cell) {
                    auto ret = cell;
                    if (ret == expected) {
                        cell = desired;
                    }
                    return ret;
                }
        };
    }
    void operator()(const FaiInstruction& instruction) {
        result = RmwLabel{
            instruction.mode,
            registers.GetRegisterValue(instruction.addr),
            [inc = registers.GetRegisterValue(instruction.increment)] (uint64_t &cell) {
                auto ret = cell;
                cell += inc;
                return ret;
            }
        };
    }
    void operator()(const LoadInstruction& instruction) {
        result = ReadLabel{
            instruction.mode,
            registers.GetRegisterValue(instruction.addr)
        };
    }
    void operator()(const StoreInstruction& instruction) {
        result = WriteLabel{
            instruction.mode,
            registers.GetRegisterValue(instruction.src),
            registers.GetRegisterValue(instruction.addr)
        };
    }
    void operator()(const FenceInstruction& instruction) {
        result = FenceLabel{instruction.mode};
    }
    void operator()(const RegisterConstantAssignment& instruction) {
        result = EpsilonLabel{};
    }
    void operator()(const RegisterBinOpAssignment& instruction) {
        result = EpsilonLabel{};
    }
    void operator()(const IfInstruction& instruction) {
        result = EpsilonLabel{};
    }
};

MemoryTransitionLabel GetTransitionLabelByInstruction(const Instruction& instruction, const ThreadLocalStorage& registers) {
    InstructionToLabelConverter converter{registers};
    std::visit(converter, instruction);
    return converter.result;
}