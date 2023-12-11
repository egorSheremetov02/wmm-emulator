#ifndef THREAD_SUBSYSTEM_H
#define THREAD_SUBSYSTEM_H
#include "../instruction/instruction.h"
#include "../common/program_descriptor.h"
#include "../thread_local_storage.h"

#include <vector>
#include <string>
#include <iostream>
#include <ostream>
#include <optional>

struct Thread {
    Thread(const ProgramDescriptor& descriptor, size_t thread_id, size_t instruction_pointer = 0);

    bool IsCompleted() const;

    void Print(std::ostream& os = std::cout, size_t indent = 0) const;
    void PrintNextInstruction(std::ostream& os = std::cout, size_t indent = 0) const;
    void PrintRegistersState(std::ostream& os = std::cout, size_t indent = 0) const;

    uint64_t GetLocalValue(Register reg) const;
    void SetLocalValue(Register reg, uint64_t value);

    void AdvanceInstructionPointer();
    void MoveInstructionPointer(size_t where);
    Instruction GetNextInstruction() const;

    const ThreadLocalStorage& GetRegisters() const;

private:
    const std::vector<Instruction>& instructions_;
    const std::vector<std::string>& instructions_str_;
    ThreadLocalStorage registers_;
    size_t instruction_pointer_ = 0;
    size_t thread_id_;
};

struct ThreadSubsystem {
    ThreadSubsystem(const ProgramDescriptor& descriptor, const std::vector<size_t>& instruction_pointers);

    std::vector<size_t> GetRunningThreads() const;
    bool IsCompleted() const;

    void Print(std::ostream& os, size_t indent = 0) const;

    std::vector<Thread> threads;

    Thread& operator[](size_t i);
    const Thread& operator[](size_t i) const;
};

#endif //THREAD_SUBSYSTEM_H
