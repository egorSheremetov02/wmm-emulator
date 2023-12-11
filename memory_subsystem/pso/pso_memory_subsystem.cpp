#include "pso_memory_subsystem.h"
#include "../memory_subsystem.h"

#include <ostream>
#include <vector>
#include <string>
#include <deque>

struct PsoPropagate : PropagateDescription {
    size_t tid;
    MemoryCell memory_cell;
    const std::vector<std::string> &memory_name;
    const std::deque<uint64_t> &cell_propagates;

    PsoPropagate(size_t tid, MemoryCell cell, const std::vector<std::string> &memory_name, const std::deque<uint64_t> &cell_propagates)
        : tid(tid)
        , memory_cell(cell)
        , memory_name(memory_name)
        , cell_propagates(cell_propagates) {

    }

    void Print(std::ostream& os, size_t indent = 0) const override {
        os << Indent{indent} << "Propagate in thread#" << tid << " of memory cell ";
        auto value = cell_propagates.front();
        if (memory_cell < memory_name.size()) {
            os << memory_name[memory_cell];
        } else {
            os << '#' << memory_cell;
        }
        os << " with a new value " << value << '\n';
    }
};

PsoMemorySubsystem::PsoMemorySubsystem(const ProgramDescriptor& descriptor, size_t threads_cnt)
    : global_memory_(descriptor.memory_size)
    , memory_name_(descriptor.memory_name)
    , pso_buffers_(threads_cnt, std::vector<std::deque<uint64_t>>(descriptor.memory_size)) {

}

PsoMemorySubsystem::PsoMemorySubsystem(
        std::vector<uint64_t> global_memory,
        const std::vector<std::string>& memory_name,
        std::vector<PsoBuffer> pso_buffers
)
        : global_memory_(std::move(global_memory))
        , memory_name_(memory_name)
        , pso_buffers_(std::move(pso_buffers)) {

}

std::vector<std::unique_ptr<PropagateDescription>> PsoMemorySubsystem::GetAvailablePropagations() const {
    std::vector<std::unique_ptr<PropagateDescription>> propagate_options;
    for (size_t tid = 0; tid < pso_buffers_.size(); ++tid) {
        for (MemoryCell cell = 0; cell < pso_buffers_[tid].size(); ++cell) {
            if (pso_buffers_[tid][cell].empty()) {
                continue;
            }
            propagate_options.push_back(std::make_unique<PsoPropagate>(tid, cell, memory_name_, pso_buffers_[tid][cell]));
        }
    }
    return propagate_options;
}

void PsoMemorySubsystem::MakePropagation(const std::unique_ptr<PropagateDescription>& propagate_description) {
    auto& pso_propagate = *static_cast<PsoPropagate *>(propagate_description.get());
    auto value = pso_propagate.cell_propagates.front();
    pso_buffers_[pso_propagate.tid][pso_propagate.memory_cell].pop_front();
    global_memory_[pso_propagate.memory_cell] = value;
}

uint64_t PsoMemorySubsystem::MakeReadTransition(size_t thread_id, ReadLabel read_label) {
    if (pso_buffers_[thread_id][read_label.src].empty()) {
        return global_memory_[read_label.src];
    }
    return pso_buffers_[thread_id][read_label.src].back();
}

void PsoMemorySubsystem::MakeWriteTransition(size_t thread_id, WriteLabel write_label) {
    pso_buffers_[thread_id][write_label.dst].push_back(write_label.value);
    if (write_label.mode == AccessMode::SEQ_CST) {
        MakeFenceTransition(thread_id, FenceLabel{AccessMode::SEQ_CST});
    }
}

void PsoMemorySubsystem::MakeFenceTransition(size_t thread_id, FenceLabel fence_label) {
    if (fence_label.mode == AccessMode::RLX) { // fences with relaxed accesses are no op
        return;
    }
    auto propagations = GetAvailablePropagations();
    while (!propagations.empty()) {
        for (auto &propagation : propagations) {
            MakePropagation(propagation);
        }
        propagations = GetAvailablePropagations();
    }
}

uint64_t PsoMemorySubsystem::MakeRmwTransition(size_t thread_id, RmwLabel rmw_label) {
    MakeFenceTransition(thread_id, FenceLabel{AccessMode::SEQ_CST});
    return rmw_label.modification(global_memory_[rmw_label.src]);
}

void PsoMemorySubsystem::Print(std::ostream& os, size_t indent) const {
    os << Indent{indent} << "PSO Memory:\n";
    os << Indent{indent + 1} << "Main memory:\n";
    for (size_t i = 0; i < memory_name_.size(); ++i) {
        os << Indent{indent + 2} << memory_name_[i] << ": " << global_memory_[i] << '\n';
    }
    for (size_t i = memory_name_.size(); i < global_memory_.size(); ++i) {
        os << Indent{indent + 2} << i << ": " << global_memory_[i] << '\n';
    }
    os << Indent{indent + 1} << "PSO buffers:\n";
    for (size_t i = 0; i < pso_buffers_.size(); ++i) {
        os << Indent{indent + 2} << "Store buffer #" << i << '\n';
        for (size_t j = 0; j < pso_buffers_[i].size(); ++j) {
            if (pso_buffers_[i][j].empty()) continue;
            os << Indent{indent + 3} << "Memory cell ";
            if (j < memory_name_.size()) {
                os << memory_name_[j];
            } else {
                os << '#' << j;
            }

            os << " store buffer: ";
            for (auto val : pso_buffers_[i][j]) {
                os << '<' << val << '>' << ' ';
            }
            os << '\n';
        }
    }
}
std::unique_ptr<MemorySubsystem> PsoMemorySubsystem::Clone() const {
    return std::make_unique<PsoMemorySubsystem>(global_memory_, memory_name_, pso_buffers_);
}