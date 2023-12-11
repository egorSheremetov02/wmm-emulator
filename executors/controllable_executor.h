#ifndef CONTROLLABLE_EXECUTOR_H
#define CONTROLLABLE_EXECUTOR_H
#include "../memory_subsystem/memory_subsystem.h"
#include "../memory_subsystem/memory_transition_labels.h"
#include "../thread_subsystem/thread_subsystem.h"
#include "../utility/print_util.h"
#include "../common/program_descriptor.h"
#include <memory>

using MemorySubsystemPtr = std::unique_ptr<MemorySubsystem>;

struct ControllableExecutor {
    std::vector<size_t> GetThreadsNextPossibleSteps() const;

    std::vector<std::unique_ptr<PropagateDescription>> GetPropagateTransitions() const;

    void MakeThreadStep(size_t thread_id);

    void MakePropagateStep(const std::unique_ptr<PropagateDescription> &);

    void SelectTransition(size_t selection, const std::vector<size_t>& running_threads, const std::vector<std::unique_ptr<PropagateDescription>>& eps_transitions);

    void PrintInstruction(std::ostream& os, size_t thread_id, size_t indent = 0) const;

    void PrintSystemSnapshot(std::ostream& os, size_t indent = 0) const;

    ControllableExecutor Clone() const;

    friend struct InstructionExecutor;

    friend ControllableExecutor CreateControllableExecutor(MemorySubsystemPtr memory_subsystem, const ProgramDescriptor& descriptor, const std::vector<size_t>& instruction_pointers);

private:
    ControllableExecutor(ThreadSubsystem thread_subsystems, const MemorySubsystemPtr& memory_ptr);

    ControllableExecutor(ThreadSubsystem thread_subsystems, MemorySubsystemPtr&& memory_ptr);

    ThreadSubsystem thread_subsystem_;
    MemorySubsystemPtr memory_subsystem_;
};

ControllableExecutor CreateControllableExecutor(MemorySubsystemPtr memory_subsystem, const ProgramDescriptor& descriptor, const std::vector<size_t>& instruction_pointers);

#endif //CONTROLLABLE_EXECUTOR_H