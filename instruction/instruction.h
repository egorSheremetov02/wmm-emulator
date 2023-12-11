#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "access_mode.h"
#include "../common/memory_primitives.h"
#include "../parser/tokenizer.h"

#include <variant>

struct CasInstruction {
    AccessMode mode;
    Register dst;
    Register addr;
    Register expected;
    Register desired;
};

struct FaiInstruction {
    AccessMode mode;
    Register dst;
    Register addr;
    Register increment;
};

struct LoadInstruction {
    AccessMode mode;
    Register dst;
    Register addr;
};

struct StoreInstruction {
    AccessMode mode;
    Register addr;
    Register src;
};

struct FenceInstruction {
    AccessMode mode;
};

struct RegisterConstantAssignment {
    Register dst;
    uint64_t value;
};

struct RegisterBinOpAssignment {
    Register dst;
    Register lhs;
    Register rhs;
    BinOp op;
};

struct IfInstruction {
    Register cond;
    size_t instr_on_success;
};

using Instruction = std::variant<CasInstruction, LoadInstruction, StoreInstruction, FaiInstruction, FenceInstruction, RegisterConstantAssignment, RegisterBinOpAssignment, IfInstruction>;

#endif //INSTRUCTION_H