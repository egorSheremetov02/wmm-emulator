#include "random_executor.h"

#include <random>

RandomExecutor::RandomExecutor(ControllableExecutor controllable_executor, bool tracing_on)
    : UserExecutor(std::move(controllable_executor), tracing_on) {

}

size_t RandomExecutor::Select() const {
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    size_t options_cnt = controllable_executor.GetPropagateTransitions().size() + controllable_executor.GetThreadsNextPossibleSteps().size();
    return std::uniform_int_distribution<size_t>(0, options_cnt - 1)(rng);
}

std::unique_ptr<UserExecutor> CreateRandomExecutor(
        MemorySubsystemPtr memory_subsystem,
        const ProgramDescriptor& descriptor,
        const std::vector<size_t>& instruction_pointers,
        bool tracing_on
) {
    ControllableExecutor controllable_executor = CreateControllableExecutor(std::move(memory_subsystem), descriptor, instruction_pointers);
    return std::make_unique<RandomExecutor>(std::move(controllable_executor), tracing_on);
}