#include "thread_subsystem.h"
#include "../utility/print_util.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ThreadSubsystem::ThreadSubsystem(const ProgramDescriptor& descriptor, const std::vector<size_t>& instruction_pointers) {
    for (size_t i = 0; i < instruction_pointers.size(); ++i) {
        threads.emplace_back(descriptor, i, instruction_pointers[i]);
    }
}

bool ThreadSubsystem::IsCompleted() const {
    return std::all_of(threads.begin(), threads.end(), [] (const Thread& thread) {
        return thread.IsCompleted();
    });
}

std::vector<size_t> ThreadSubsystem::GetRunningThreads() const {
    std::vector<size_t> running_threads;
    for (size_t i = 0; i < threads.size(); ++i) {
        if (!threads[i].IsCompleted()) {
            running_threads.push_back(i);
        }
    }
    return running_threads;
}

Thread& ThreadSubsystem::operator[](size_t tid) {
    return threads[tid];
}

const Thread& ThreadSubsystem::operator[](size_t tid) const {
    return threads[tid];
}

void ThreadSubsystem::Print(std::ostream& os, size_t indent) const {
    os << Indent{indent} << "Threads info:\n";
    for (auto &thread: threads) {
        thread.Print(os, indent + 1);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Thread::Thread(const ProgramDescriptor& descriptor, size_t thread_id, size_t instruction_pointer)
    : instructions_(descriptor.instructions)
    , instructions_str_(descriptor.instructions_str)
    , registers_(descriptor.register_name)
    , instruction_pointer_(instruction_pointer)
    , thread_id_(thread_id) {
}

bool Thread::IsCompleted() const {
    return instruction_pointer_ == instructions_.size();
}

void Thread::PrintNextInstruction(std::ostream& os, size_t indent) const {
    if (IsCompleted()) {
        os << Indent{indent} << "Instructions are completed\n";
    } else {
        os << Indent{indent} << instructions_str_[instruction_pointer_] << '\n';
    }
}

void Thread::PrintRegistersState(std::ostream &os, size_t indent) const {
    registers_.Print(os, indent);
}

void Thread::Print(std::ostream& os, size_t indent) const {
    os << Indent{indent} << "Thread #" << thread_id_ << '\n';
    os << Indent{indent + 1} << "Next instruction is: ";
    PrintNextInstruction(os, 0);
    PrintRegistersState(os, indent + 1);
}

uint64_t Thread::GetLocalValue(Register reg) const {
    return registers_.GetRegisterValue(reg);
}

void Thread::SetLocalValue(Register reg, uint64_t value) {
    registers_.SetRegisterValue(reg, value);
}

void Thread::AdvanceInstructionPointer() {
    ++instruction_pointer_;
}
void Thread::MoveInstructionPointer(size_t where) {
    instruction_pointer_ = where;
}
Instruction Thread::GetNextInstruction() const {
    assert(!IsCompleted());
    return instructions_[instruction_pointer_];
}

const ThreadLocalStorage& Thread::GetRegisters() const {
    return registers_;
}
