#include "user_executor.h"

#include <iostream>

UserExecutor::UserExecutor(ControllableExecutor controllable_executor, bool tracing_on)
    : controllable_executor(std::move(controllable_executor))
    , tracing_on(tracing_on) {

}

bool UserExecutor::IsDone() const {
    auto eps_cnt = controllable_executor.GetPropagateTransitions().size();
    auto threads_running = controllable_executor.GetThreadsNextPossibleSteps().size();
    return eps_cnt + threads_running == 0;
}

void UserExecutor::ExecuteNext() {
    if (tracing_on) {
        controllable_executor.PrintSystemSnapshot(std::cout);
    }
    size_t next_index = Select();
    controllable_executor.SelectTransition(
            next_index,
            controllable_executor.GetThreadsNextPossibleSteps(),
            controllable_executor.GetPropagateTransitions()
    );
}

void UserExecutor::PrintSnapshot() {
    controllable_executor.PrintSystemSnapshot(std::cout);
}