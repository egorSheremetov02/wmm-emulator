#ifndef MEMORY_SUBSYSTEM_H
#define MEMORY_SUBSYSTEM_H
#include <cstddef>
#include <ostream>
#include <vector>
#include <memory>

#include "memory_transition_labels.h"

struct PropagateDescription {
    virtual void Print(std::ostream& os, size_t indent = 0) const = 0;

    virtual ~PropagateDescription() = default;
};

struct MemorySubsystem {
    virtual std::vector<std::unique_ptr<PropagateDescription>> GetAvailablePropagations() const = 0;
    virtual void MakePropagation(const std::unique_ptr<PropagateDescription>& propagate_description) = 0;
    virtual uint64_t MakeReadTransition(size_t thread_id, ReadLabel read_label) = 0;
    virtual void MakeWriteTransition(size_t thread_id, WriteLabel write_label) = 0;
    virtual void MakeFenceTransition(size_t thread_id, FenceLabel fence_label) = 0;
    virtual uint64_t MakeRmwTransition(size_t thread_id, RmwLabel rmw_label) = 0;
    virtual void Print(std::ostream& os, size_t indent = 0) const = 0;
    virtual std::unique_ptr<MemorySubsystem> Clone() const = 0;
    virtual ~MemorySubsystem() = default;
};

#endif //MEMORY_SUBSYSTEM_H