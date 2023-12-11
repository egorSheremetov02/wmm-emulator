#ifndef MC_EXECUTOR_H
#define MC_EXECUTOR_H
#include "user_executor.h"

struct McExecutor : UserExecutor {
    size_t cur_transition = 0;

    McExecutor(ControllableExecutor controllable_executor, bool tracing_on);

    bool IsDone() const override;
    size_t Select() const override;
    void ExecuteNext() override;
};

std::unique_ptr<UserExecutor> CreateModelCheckingExecutor(
        MemorySubsystemPtr memory_subsystem,
        const ProgramDescriptor& descriptor,
        const std::vector<size_t>& instruction_pointers,
        bool tracing_on
);

#endif //MC_EXECUTOR_H