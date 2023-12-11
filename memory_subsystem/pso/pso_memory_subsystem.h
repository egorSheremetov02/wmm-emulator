#ifndef PSO_MEMORY_SUBSYSTEM_H
#define PSO_MEMORY_SUBSYSTEM_H
#include "../memory_subsystem.h"
#include "../../common/program_descriptor.h"

#include <vector>
#include <deque>

using PsoBuffer = std::vector<std::deque<uint64_t>>;

struct PsoMemorySubsystem : MemorySubsystem {
    PsoMemorySubsystem(const ProgramDescriptor& descriptor, size_t threads_cnt);
    PsoMemorySubsystem(std::vector<uint64_t> global_memory, const std::vector<std::string>& memory_name, std::vector<PsoBuffer> pso_buffers);

    std::vector<std::unique_ptr<PropagateDescription>> GetAvailablePropagations() const override;
    void MakePropagation(const std::unique_ptr<PropagateDescription>& propagate_description) override;
    uint64_t MakeReadTransition(size_t thread_id, ReadLabel read_label) override;
    void MakeWriteTransition(size_t thread_id, WriteLabel write_label) override;
    void MakeFenceTransition(size_t thread_id, FenceLabel fence_label) override;
    uint64_t MakeRmwTransition(size_t thread_id, RmwLabel rmw_label) override;
    void Print(std::ostream& os, size_t indent = 0) const override;
    std::unique_ptr<MemorySubsystem> Clone() const override;
private:
    std::vector<uint64_t> global_memory_;
    const std::vector<std::string>& memory_name_;
    std::vector<PsoBuffer> pso_buffers_;
};

#endif //PSO_MEMORY_SUBSYSTEM_H