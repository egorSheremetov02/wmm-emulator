#include "parser/parser.h"
#include "executors/user_executor.h"
#include "executors/random_executor.h"
#include "executors/interactive_executor.h"
#include "executors/mc_executor.h"
#include "memory_subsystem/sc/sc_memory_subsystem.h"
#include "memory_subsystem/tso/tso_memory_subsystem.h"
#include "memory_subsystem/pso/pso_memory_subsystem.h"

#include <iostream>
#include <fstream>
#include <string>

MemorySubsystemPtr CreateMemorySubsystem(const ProgramDescriptor& descriptor, size_t threads_cnt, std::string operational_model) {
    if (operational_model == "sc") {
        return std::make_unique<ScMemorySubsystem>(descriptor, threads_cnt);
    } else if (operational_model == "tso") {
        return std::make_unique<TsoMemorySubsystem>(descriptor, threads_cnt);
    } else if (operational_model == "pso") {
        return std::make_unique<PsoMemorySubsystem>(descriptor, threads_cnt);
    } else {
        throw std::runtime_error{"Unknown operational model, there is no implementation for it as of now"};
    }
}


int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cout << "Incorrect usage of wmm-emulator\n";
        std::cout << "Correct usage: " << argv[0] << "<input-file-path> <operational_model> <execution_mode> <tracing_mode> <instruction_pointers...>\n";
        exit(1);
    }
    std::ifstream input_file(argv[1]);
    std::string operational_model(argv[2]);
    std::string execution_mode(argv[3]);
    std::string tracing_mode(argv[4]);

    bool tracing_on = tracing_mode == "on";

    std::vector<size_t> instruction_pointers;
    for (int i = 5; i < argc; ++i) {
        size_t ip = std::stoull(argv[i]);
        instruction_pointers.push_back(ip);
    }

    if (instruction_pointers.empty()) {
        throw std::runtime_error{"Expected positive number of instruction pointers"};
    }

    ProgramDescriptor descriptor = Parse(&input_file);
    MemorySubsystemPtr memory_subsystem = CreateMemorySubsystem(descriptor, instruction_pointers.size(), operational_model);

    if (execution_mode == "model-checking") {
        throw std::runtime_error{"Model checking is not implemented yet"};
    } else {
        std::unique_ptr<UserExecutor> executor;
        if (execution_mode == "random") {
            executor = CreateRandomExecutor(std::move(memory_subsystem), descriptor, instruction_pointers, tracing_on);
        } else if (execution_mode == "interactive") {
            executor = CreateInteractiveExecutor(std::move(memory_subsystem), descriptor, instruction_pointers, tracing_on);
        } else if (execution_mode == "mc") {
            executor = CreateModelCheckingExecutor(std::move(memory_subsystem), descriptor, instruction_pointers, tracing_on);
        } else {
            throw std::runtime_error{"Unsupported execution mode"};
        }

        while (!executor->IsDone()) {
            executor->ExecuteNext();
        }
        if (tracing_on && execution_mode != "mc") {
            executor->PrintSnapshot();
        }
    }

    return 0;
}
