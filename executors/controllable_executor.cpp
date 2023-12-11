#include "controllable_executor.h"
#include "../memory_subsystem/memory_subsystem.h"

#include <iostream>

struct InstructionExecutor {
    size_t thread_id;
    ControllableExecutor* executor;

    void operator()(const CasInstruction& instruction) {
        Thread& cur_thread = executor->thread_subsystem_[thread_id];
        MemoryTransitionLabel mtl = GetTransitionLabelByInstruction(instruction, cur_thread.GetRegisters());
        auto value = executor->memory_subsystem_->MakeRmwTransition(thread_id, std::get<RmwLabel>(mtl));
        cur_thread.SetLocalValue(instruction.dst, value);
        cur_thread.AdvanceInstructionPointer();
    }
    void operator()(const FaiInstruction& instruction) {
        Thread& cur_thread = executor->thread_subsystem_[thread_id];
        MemoryTransitionLabel mtl = GetTransitionLabelByInstruction(instruction, cur_thread.GetRegisters());
        auto value = executor->memory_subsystem_->MakeRmwTransition(thread_id, std::get<RmwLabel>(mtl));
        cur_thread.SetLocalValue(instruction.dst, value);
        cur_thread.AdvanceInstructionPointer();
    }
    void operator()(const LoadInstruction& instruction) {
        Thread& cur_thread = executor->thread_subsystem_[thread_id];
        MemoryTransitionLabel mtl = GetTransitionLabelByInstruction(instruction, cur_thread.GetRegisters());
        auto value = executor->memory_subsystem_->MakeReadTransition(thread_id, std::get<ReadLabel>(mtl));
        cur_thread.SetLocalValue(instruction.dst, value);
        cur_thread.AdvanceInstructionPointer();
    }
    void operator()(const StoreInstruction& instruction) {
        Thread& cur_thread = executor->thread_subsystem_[thread_id];
        MemoryTransitionLabel mtl = GetTransitionLabelByInstruction(instruction, cur_thread.GetRegisters());
        executor->memory_subsystem_->MakeWriteTransition(thread_id, std::get<WriteLabel>(mtl));
        cur_thread.AdvanceInstructionPointer();
    }
    void operator()(const FenceInstruction& instruction) {
        Thread& cur_thread = executor->thread_subsystem_[thread_id];
        MemoryTransitionLabel mtl = GetTransitionLabelByInstruction(instruction, cur_thread.GetRegisters());
        executor->memory_subsystem_->MakeFenceTransition(thread_id, std::get<FenceLabel>(mtl));
        cur_thread.AdvanceInstructionPointer();
    }
    void operator()(const RegisterConstantAssignment& instruction) {
        Thread& cur_thread = executor->thread_subsystem_[thread_id];
        cur_thread.SetLocalValue(instruction.dst, instruction.value);
        cur_thread.AdvanceInstructionPointer();
    }
    void operator()(const RegisterBinOpAssignment& instruction) {
        Thread &cur_thread = executor->thread_subsystem_[thread_id];
        uint64_t lhs_value = cur_thread.GetLocalValue(instruction.lhs);
        uint64_t rhs_value = cur_thread.GetLocalValue(instruction.rhs);
        uint64_t res_value;
        switch (instruction.op) {
            case ADD:
                res_value = lhs_value + rhs_value;
                break;
            case SUBTRACT:
                res_value = lhs_value - rhs_value;
                break;
            case DIVIDE:
                if (rhs_value == 0) {
                    throw std::runtime_error{"Division by zero is not allowed"};
                }
                res_value = lhs_value / rhs_value;
                break;
            case MULTIPLY:
                res_value = lhs_value * rhs_value;
                break;
            case LESS:
                res_value = lhs_value < rhs_value;
                break;
            case GREATER:
                res_value = lhs_value > rhs_value;
                break;
            case LESS_EQUAL:
                res_value = lhs_value <= rhs_value;
                break;
            case GREATER_EQUAL:
                res_value = lhs_value >= rhs_value;
                break;
            default:
                assert(false);
        }
        cur_thread.SetLocalValue(instruction.dst, res_value);
        cur_thread.AdvanceInstructionPointer();
    }
    void operator()(const IfInstruction& instruction) {
        Thread& cur_thread = executor->thread_subsystem_[thread_id];
        uint64_t cond_value = cur_thread.GetLocalValue(instruction.cond);
        if (cond_value == 0) {
            cur_thread.AdvanceInstructionPointer();
        } else {
            cur_thread.MoveInstructionPointer(instruction.instr_on_success);
        }
    }
};

void ControllableExecutor::MakePropagateStep(const std::unique_ptr<PropagateDescription>& propagate_description) {
    memory_subsystem_->MakePropagation(propagate_description);
}

std::vector<std::unique_ptr<PropagateDescription>> ControllableExecutor::GetPropagateTransitions() const {
    return memory_subsystem_->GetAvailablePropagations();
}

void ControllableExecutor::MakeThreadStep(size_t tid) {
    std::visit(InstructionExecutor{tid, this}, thread_subsystem_[tid].GetNextInstruction());
}

std::vector<size_t> ControllableExecutor::GetThreadsNextPossibleSteps() const {
    return thread_subsystem_.GetRunningThreads();
}

void ControllableExecutor::SelectTransition(
        size_t selection,
        const std::vector<size_t>& running_threads,
        const std::vector<std::unique_ptr<PropagateDescription>>& eps_transitions
) {
    assert(selection < running_threads.size() + eps_transitions.size());
    if (selection < running_threads.size()) {
        MakeThreadStep(running_threads[selection]);
    } else {
        MakePropagateStep(eps_transitions[selection - running_threads.size()]);
    }
}

void ControllableExecutor::PrintInstruction(std::ostream& os, size_t thread_id, size_t indent) const {
    thread_subsystem_[thread_id].PrintNextInstruction(os, indent);
}

void ControllableExecutor::PrintSystemSnapshot(std::ostream& os, size_t indent) const {
    thread_subsystem_.Print(os, indent);
    memory_subsystem_->Print(os, indent);
    os << '\n';
}

ControllableExecutor ControllableExecutor::Clone() const {
    return ControllableExecutor{thread_subsystem_, memory_subsystem_->Clone()};
}

ControllableExecutor::ControllableExecutor(ThreadSubsystem thread_subsystem, const MemorySubsystemPtr& memory_ptr)
    : thread_subsystem_(std::move(thread_subsystem))
    , memory_subsystem_(memory_ptr->Clone()) {

}

ControllableExecutor::ControllableExecutor(ThreadSubsystem thread_subsystem, MemorySubsystemPtr&& memory_ptr)
    : thread_subsystem_(std::move(thread_subsystem))
    , memory_subsystem_(std::move(memory_ptr)) {

}

ControllableExecutor CreateControllableExecutor(MemorySubsystemPtr memory_subsystem, const ProgramDescriptor& descriptor, const std::vector<size_t>& instruction_pointers) {
    ThreadSubsystem thread_subsystem(descriptor, instruction_pointers);
    return ControllableExecutor(std::move(thread_subsystem), std::move(memory_subsystem));
}
