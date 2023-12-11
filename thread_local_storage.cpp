#include "thread_local_storage.h"

ThreadLocalStorage::ThreadLocalStorage(const std::vector<std::string>& register_name)
        : value_(register_name.size(), 0)
        , register_name_(register_name)
{}

uint64_t ThreadLocalStorage::GetRegisterValue(Register reg) const {
    if (reg >= value_.size()) {
        throw std::runtime_error{"Tried to access an invalid register"};
    }
    return value_[reg];
}

void ThreadLocalStorage::SetRegisterValue(Register reg, uint64_t val) {
    if (reg >= value_.size()) {
        throw std::runtime_error{"Tried to access an invalid register"};
    }
    value_[reg] = val;
}

void ThreadLocalStorage::Print(std::ostream& os, size_t indent) const {
    os << Indent{indent} << "Registers' state:\n";
    for (size_t i = 0; i < value_.size(); ++i) {
        os << Indent{indent + 1} << register_name_[i] << ": " << value_[i] << '\n';
    }
}

std::ostream& operator<<(std::ostream& os, const ThreadLocalStorage& local_storage) {
    local_storage.Print(os);
    return os;
}