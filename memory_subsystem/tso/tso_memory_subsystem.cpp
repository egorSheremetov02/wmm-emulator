#include "tso_memory_subsystem.h"
#include "../../utility/print_util.h"

#include <ostream>

struct TsoPropagate : PropagateDescription {
    size_t tid;
    const std::vector<std::string>& memory_name;
    const StoreBuffer& store_buffer;

    TsoPropagate(size_t tid, const std::vector<std::string>& memory_name, const StoreBuffer& store_buffer)
        : tid(tid)
        , memory_name(memory_name)
        , store_buffer(store_buffer) {
    }

    void Print(std::ostream &os, size_t indent = 0) const override {
        os << Indent{indent} << "Propagate in thread#" << tid << " of memory cell ";
        auto [cell, value] = store_buffer.front();
        if (cell < memory_name.size()) {
            os << memory_name[cell];
        } else {
            os << '#' << cell;
        }
        os << " with a new value " << value << '\n';
    }
};

TsoMemorySubsystem::TsoMemorySubsystem(const ProgramDescriptor& descriptor, [[maybe_unused]] size_t threads_cnt)
        : global_memory_(descriptor.memory_size), memory_name_(descriptor.memory_name), store_buffers_(threads_cnt) {

}

std::vector<std::unique_ptr<PropagateDescription>> TsoMemorySubsystem::GetAvailablePropagations() const {
    std::vector<std::unique_ptr<PropagateDescription>> propagate_options;
    for (size_t tid = 0; tid < store_buffers_.size(); ++tid) {
        auto& buffer = store_buffers_[tid];
        if (!buffer.empty()) {
            propagate_options.push_back(std::make_unique<TsoPropagate>(tid, memory_name_, buffer));
        }
    }
    return propagate_options;
}

void TsoMemorySubsystem::MakePropagation(const std::unique_ptr<PropagateDescription>& propagate_description) {
    auto& tso_propagate = *static_cast<TsoPropagate *>(propagate_description.get());
    auto [cell, value] = store_buffers_[tso_propagate.tid].front();
    store_buffers_[tso_propagate.tid].pop_front();
    global_memory_[cell] = value;
}

uint64_t TsoMemorySubsystem::MakeReadTransition(size_t thread_id, ReadLabel read_label) {
    if (store_buffers_[thread_id].empty()) {
        return global_memory_[read_label.src];
    }
    // find first value from the back
    for (size_t i = store_buffers_[thread_id].size(); i > 0; --i) {
        if (store_buffers_[thread_id][i].first == read_label.src) {
            return store_buffers_[thread_id][i].second;
        }
    }
    return global_memory_[read_label.src];
}

void TsoMemorySubsystem::MakeWriteTransition(size_t thread_id, WriteLabel write_label) {
    store_buffers_[thread_id].emplace_back(write_label.dst, write_label.value);
    if (write_label.mode == AccessMode::SEQ_CST) { //ensure sequential consistency by inserting fences after each write operation
        MakeFenceTransition(thread_id, FenceLabel{AccessMode::SEQ_CST});
    }
}

void TsoMemorySubsystem::MakeFenceTransition(size_t thread_id, FenceLabel fence_label) {
    auto propagations = GetAvailablePropagations();
    while (!propagations.empty()) {
        for (auto& propagation : propagations) {
            MakePropagation(std::move(propagation));
        }
        propagations = GetAvailablePropagations();
    }
}

uint64_t TsoMemorySubsystem::MakeRmwTransition(size_t thread_id, RmwLabel rmw_label) {
    MakeFenceTransition(thread_id, FenceLabel{AccessMode::SEQ_CST});
    return rmw_label.modification(global_memory_[rmw_label.src]);
}

void TsoMemorySubsystem::Print(std::ostream& os, size_t indent) const {
    os << Indent{indent} << "TSO Memory:\n";
    os << Indent{indent + 1} << "Main memory:\n";
    for (size_t i = 0; i < memory_name_.size(); ++i) {
        os << Indent{indent + 2} << memory_name_[i] << ": " << global_memory_[i] << '\n';
    }
    for (size_t i = memory_name_.size(); i < global_memory_.size(); ++i) {
        os << Indent{indent + 2} << i << ": " << global_memory_[i] << '\n';
    }
    os << Indent{indent + 1} << "Store buffers:\n";
    for (size_t i = 0; i < store_buffers_.size(); ++i) {
        os << Indent{indent + 2} << "Store buffer #" << i << '\n';
        for (size_t j = 0; j < store_buffers_[i].size(); ++j) {
            os << Indent{indent + 3};
            if (store_buffers_[i][j].first < memory_name_.size()) {
                os << "<" << memory_name_[store_buffers_[i][j].first] << ", " << store_buffers_[i][j].second << ">" << '\n';
            } else {
                os << "<" << store_buffers_[i][j].first << ", " << store_buffers_[i][j].second << ">" << '\n';
            }
        }
    }
}

std::unique_ptr<MemorySubsystem> TsoMemorySubsystem::Clone() const {
    return std::make_unique<TsoMemorySubsystem>(global_memory_, memory_name_, store_buffers_);
}

TsoMemorySubsystem::TsoMemorySubsystem(
        std::vector<uint64_t> global_memory,
        const std::vector<std::string>& memory_name,
        std::vector<std::deque<std::pair<MemoryCell, uint64_t>>> store_buffers
)
        : global_memory_(std::move(global_memory))
        , memory_name_(memory_name)
        , store_buffers_(std::move(store_buffers)) {

}
