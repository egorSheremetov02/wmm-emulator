#ifndef MEMORY_TRANSITION_LABELS_H
#define MEMORY_TRANSITION_LABELS_H
#include "../common/memory_primitives.h"
#include "../instruction/access_mode.h"
#include "../instruction/instruction.h"
#include "../thread_local_storage.h"

#include <cstddef>
#include <inttypes.h>
#include <functional>
#include <optional>
#include <variant>

using RmwAtomicOperation = std::function<uint64_t(uint64_t&)>;

struct WriteLabel {
    AccessMode mode;
    uint64_t value;
    MemoryCell dst;
};

struct ReadLabel {
    AccessMode mode;
    MemoryCell src;
};

struct RmwLabel {
    AccessMode mode;
    MemoryCell src;
    RmwAtomicOperation modification;
};

struct FenceLabel {
    AccessMode mode;
};

struct EpsilonLabel {
};

using MemoryTransitionLabel = std::variant<FenceLabel, RmwLabel, ReadLabel, WriteLabel, EpsilonLabel>;

MemoryTransitionLabel GetTransitionLabelByInstruction(const Instruction& instruction, const ThreadLocalStorage& registers);

#endif //MEMORY_TRANSITION_LABELS_H