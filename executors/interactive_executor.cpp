#include "interactive_executor.h"

#include <random>
#include <iostream>

InteractiveExecutor::InteractiveExecutor(ControllableExecutor controllable_executor, bool tracing_on)
        : UserExecutor(std::move(controllable_executor), tracing_on) {

}

size_t InteractiveExecutor::Select() const {
    auto thread_transitions = controllable_executor.GetThreadsNextPossibleSteps();
    auto epsilon_transitions_tids = controllable_executor.GetPropagateTransitions();
    std::cout << "Transition options: \n";
    for (size_t i = 0; i < thread_transitions.size() + epsilon_transitions_tids.size(); ++i) {
        std::cout << Indent{1} << i << ". ";
        if (i < thread_transitions.size()) {
            std::cout << Indent{1} << "Next instruction in thread#" << i << ": ";
            controllable_executor.PrintInstruction(std::cout, thread_transitions[i], 0);
        } else {
            epsilon_transitions_tids[i - thread_transitions.size()]->Print(std::cout, 1);
        }
        std::cout.flush();
    }
    size_t selection;
    std::cout << "Please enter the index of a next transition: ";
    if (!(std::cin >> selection)) {
        throw std::runtime_error{"Incorrect user input, wrong format"};
    }
    if (selection > thread_transitions.size() + epsilon_transitions_tids.size()) {
        throw std::runtime_error{"Incorrect user input, out of range"};
    }
    return selection;
}

std::unique_ptr<UserExecutor> CreateInteractiveExecutor(MemorySubsystemPtr memory_subsystem, const ProgramDescriptor& descriptor, const std::vector<size_t>& instruction_pointers, bool tracing_on) {
    ControllableExecutor controllable_executor = CreateControllableExecutor(std::move(memory_subsystem), descriptor, instruction_pointers);
    return std::make_unique<InteractiveExecutor>(std::move(controllable_executor), tracing_on);
}