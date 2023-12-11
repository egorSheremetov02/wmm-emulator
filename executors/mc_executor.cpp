#include "mc_executor.h"

McExecutor::McExecutor(ControllableExecutor controllable_executor, bool tracing_on)
    : UserExecutor(std::move(controllable_executor), tracing_on) {

}

std::unique_ptr<UserExecutor> CreateModelCheckingExecutor(
        MemorySubsystemPtr memory_subsystem,
        const ProgramDescriptor& descriptor,
        const std::vector<size_t>& instruction_pointers,
        bool tracing_on
) {
    ControllableExecutor controllable_executor = CreateControllableExecutor(std::move(memory_subsystem), descriptor, instruction_pointers);
    return std::make_unique<McExecutor>(std::move(controllable_executor), tracing_on);
}

bool McExecutor::IsDone() const {
    return cur_transition == controllable_executor.GetThreadsNextPossibleSteps().size() + controllable_executor.GetPropagateTransitions().size();
}

size_t McExecutor::Select() const {
    return cur_transition;
}

void McExecutor::ExecuteNext() {
    if (tracing_on) {
        PrintSnapshot();
    }
    size_t selection = Select();
    ++cur_transition;
    auto copy = controllable_executor.Clone();
    copy.SelectTransition(
            selection,
            controllable_executor.GetThreadsNextPossibleSteps(),
            controllable_executor.GetPropagateTransitions()
    );
    McExecutor executor{std::move(copy), tracing_on};

    if (executor.IsDone()) {
        std::cout << "MC Executor final state:\n";
        PrintSnapshot();
        return;
    }

    while (!executor.IsDone()) {
        executor.ExecuteNext();
    }
}