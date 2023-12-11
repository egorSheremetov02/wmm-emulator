#ifndef PROGRAM_DESCRIPTOR_H
#define PROGRAM_DESCRIPTOR_H
#include "../instruction/instruction.h"
#include <cstddef>
#include <vector>
#include <unordered_map>

struct ProgramDescriptor {
    size_t memory_size;
    std::vector<Instruction> instructions;
    std::vector<std::string> instructions_str;
    std::vector<std::string> memory_name;
    std::vector<std::string> register_name;
};

#endif //PROGRAM_DESCRIPTOR_H