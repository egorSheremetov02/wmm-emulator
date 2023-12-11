#ifndef INTERACTIVE_EXECUTOR_H
#include "user_executor.h"

struct InteractiveExecutor : UserExecutor {
    explicit InteractiveExecutor(ControllableExecutor controllable_executor, bool tracing_on);
    size_t Select() const override;
};

std::unique_ptr<UserExecutor> CreateInteractiveExecutor(MemorySubsystemPtr memory_subsystem, const ProgramDescriptor& descriptor, const std::vector<size_t>& instruction_pointers, bool tracing_on = false);

#endif //INTERACTIVE_EXECUTOR_H