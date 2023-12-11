#ifndef TSO_MEMORY_SUBSYSTEM_H
#define TSO_MEMORY_SUBSYSTEM_H
#include "../memory_subsystem.h"
#include "../../common/program_descriptor.h"
#include "../../common/memory_primitives.h"

#include <vector>
#include <deque>

using StoreBuffer = std::deque<std::pair<MemoryCell, uint64_t>>;

struct TsoMemorySubsystem : MemorySubsystem {
    TsoMemorySubsystem(const ProgramDescriptor& descriptor, size_t threads_cnt);
    TsoMemorySubsystem(std::vector<uint64_t> global_memory, const std::vector<std::string>& memory_name, std::vector<std::deque<std::pair<MemoryCell, uint64_t>>> store_buffers);

    std::vector<std::unique_ptr<PropagateDescription>> GetAvailablePropagations() const override;
    void MakePropagation(const std::unique_ptr<PropagateDescription>& propagate_description) override;
    uint64_t MakeReadTransition(size_t thread_id, ReadLabel read_label) override;
    void MakeWriteTransition(size_t thread_id, WriteLabel write_label) override;
    void MakeFenceTransition(size_t thread_id, FenceLabel fence_label) override;
    uint64_t MakeRmwTransition(size_t thread_id, RmwLabel rmw_label) override;
    void Print(std::ostream& os, size_t indent) const override;
    std::unique_ptr<MemorySubsystem> Clone() const override;
private:
    std::vector<uint64_t> global_memory_;
    const std::vector<std::string>& memory_name_;
    std::vector<StoreBuffer> store_buffers_;
};
#endif //TSO_MEMORY_SUBSYSTEM_H