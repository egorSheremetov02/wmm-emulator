#ifndef RANDOM_EXECUTOR_H
#define RANDOM_EXECUTOR_H

#include "controllable_executor.h"
#include "user_executor.h"

#include <memory>

struct RandomExecutor : UserExecutor {
    explicit RandomExecutor(ControllableExecutor controllable_executor, bool tracing_on);
    size_t Select() const override;
};

std::unique_ptr<UserExecutor> CreateRandomExecutor(MemorySubsystemPtr memory_subsystem, const ProgramDescriptor& descriptor, const std::vector<size_t>& instruction_pointers, bool tracing_on = false);

#endif //RANDOM_EXECUTOR_H