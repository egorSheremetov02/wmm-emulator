#ifndef THREAD_LOCAL_STORAGE_H
#define THREAD_LOCAL_STORAGE_H
#include "common/memory_primitives.h"

#include "utility/print_util.h"

#include <vector>
#include <string>
#include <ostream>

struct ThreadLocalStorage {

    ThreadLocalStorage(const std::vector<std::string>& register_name);

    [[nodiscard]] uint64_t GetRegisterValue(Register reg) const;

    void SetRegisterValue(Register reg, uint64_t val);

    void Print(std::ostream& os, size_t indent = 0) const;

private:
    std::vector<uint64_t> value_;
    const std::vector<std::string>& register_name_;
};

std::ostream& operator<<(std::ostream& os, const ThreadLocalStorage& local_storage);

#endif
