#include "sc_memory_subsystem.h"
#include "../../utility/print_util.h"

ScMemorySubsystem::ScMemorySubsystem(const ProgramDescriptor& descriptor, [[maybe_unused]] size_t threads_cnt)
    : global_memory_(descriptor.memory_size), memory_name_(descriptor.memory_name) {

}

std::vector<std::unique_ptr<PropagateDescription>> ScMemorySubsystem::GetAvailablePropagations() const {
    return {};
}

// should only be invoked on one of the objects returns from GetAvailablePropagations, but for SC it is always empty
void ScMemorySubsystem::MakePropagation(const std::unique_ptr<PropagateDescription>& propagate_description) {
    throw std::runtime_error{"SC doesn't have propagations, MakePropagation was called due to some bug"};
}

uint64_t ScMemorySubsystem::MakeReadTransition(size_t thread_id, ReadLabel read_label) {
    return global_memory_[read_label.src];
}

void ScMemorySubsystem::MakeWriteTransition(size_t thread_id, WriteLabel write_label) {
    global_memory_[write_label.dst] = write_label.value;
}

void ScMemorySubsystem::MakeFenceTransition(size_t thread_id, FenceLabel fence_label) {

}

uint64_t ScMemorySubsystem::MakeRmwTransition(size_t thread_id, RmwLabel rmw_label) {
    return rmw_label.modification(global_memory_[rmw_label.src]);
}

void ScMemorySubsystem::Print(std::ostream& os, size_t indent) const {
    os << Indent{indent} << "SC Memory:\n";
    for (size_t i = 0; i < memory_name_.size(); ++i) {
        os << Indent{indent + 1} << memory_name_[i] << ": " << global_memory_[i] << '\n';
    }
    for (size_t i = memory_name_.size(); i < global_memory_.size(); ++i) {
        os << Indent{indent + 1} << i << ": " << global_memory_[i] << '\n';
    }
}

std::unique_ptr<MemorySubsystem> ScMemorySubsystem::Clone() const {
    return std::make_unique<ScMemorySubsystem>(global_memory_, memory_name_);
}

ScMemorySubsystem::ScMemorySubsystem(std::vector<uint64_t> global_memory, const std::vector<std::string>& memory_name)
    : global_memory_(std::move(global_memory))
    , memory_name_(memory_name) {

}
