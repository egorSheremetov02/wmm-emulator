#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <variant>

#include "../parser/parser.h"

TEST(TestParser, RegisterConstantAssignment) {
    std::string program = R""""(
                r = 1;
                )"""";
    std::stringstream ss{program};
    auto descriptor = Parse(&ss);
    EXPECT_EQ(descriptor.memory_size, 0);
    EXPECT_EQ(descriptor.instructions.size(), 1);
    EXPECT_TRUE(std::holds_alternative<RegisterConstantAssignment>(descriptor.instructions[0]));
}

TEST(TestParser, RegisterConstantMemoryAssignment) {
    std::string program = R""""(
                shared_state: x y z;
                r = x;
                )"""";
    std::stringstream ss{program};
    auto descriptor = Parse(&ss);
    EXPECT_EQ(descriptor.memory_size, 3);
    EXPECT_EQ(descriptor.instructions.size(), 1);
    EXPECT_TRUE(std::holds_alternative<RegisterConstantAssignment>(descriptor.instructions[0]));
}

TEST(TestParser, RegisterBinOpAssignment) {
    std::string program = R""""(
                shared_state: x y z;
                r = r1 < r2;
                )"""";
    std::stringstream ss{program};
    auto descriptor = Parse(&ss);
    EXPECT_EQ(descriptor.memory_size, 3);
    EXPECT_EQ(descriptor.instructions.size(), 1);
    EXPECT_TRUE(std::holds_alternative<RegisterBinOpAssignment>(descriptor.instructions[0]));
}

TEST(TestParser, FenceInstruction) {
    std::string program = R""""(
                fence SEQ_CST;
                )"""";
    std::stringstream ss{program};
    auto descriptor = Parse(&ss);
    EXPECT_EQ(descriptor.memory_size, 0);
    EXPECT_EQ(descriptor.instructions.size(), 1);
    EXPECT_TRUE(std::holds_alternative<FenceInstruction>(descriptor.instructions[0]));
}

TEST(TestParser, FaiInstruction) {
    std::string program = R""""(
                shared_state: x;
                r1 = x;
                r2 = 6;
                r := fai SEQ_CST #r1 r2;
                )"""";
    std::stringstream ss{program};
    auto descriptor = Parse(&ss);
    EXPECT_EQ(descriptor.memory_size, 1);
    EXPECT_EQ(descriptor.instructions.size(), 3);
    EXPECT_TRUE(std::holds_alternative<FaiInstruction>(descriptor.instructions[2]));
}

TEST(TestParser, CasInstruction) {
    std::string program = R""""(
                shared_state: x;
                reserve_space: 100;
                r1 = x;
                r2 = 6;
                store SEQ_CST #r1 r2;
                r2 = 6;
                r3 = 7;
                r := cas SEQ_CST #r1 r2 r3;
                )"""";
    std::stringstream ss{program};
    auto descriptor = Parse(&ss);
    EXPECT_EQ(descriptor.memory_size, 101);
    EXPECT_EQ(descriptor.instructions.size(), 6);
    EXPECT_TRUE(std::holds_alternative<StoreInstruction>(descriptor.instructions[2]));
    EXPECT_TRUE(std::holds_alternative<CasInstruction>(descriptor.instructions[5]));
}

TEST(TestParser, LoadInstruction) {
    std::string program = R""""(
                shared_state: x;
                r1 = x;
                r2 = 6;
                store SEQ_CST #r1 r2;
                load SEQ_CST #r1 r;
                )"""";
    std::stringstream ss{program};
    auto descriptor = Parse(&ss);
    EXPECT_EQ(descriptor.memory_size, 1);
    EXPECT_EQ(descriptor.instructions.size(), 4);
    EXPECT_TRUE(std::holds_alternative<StoreInstruction>(descriptor.instructions[2]));
    EXPECT_TRUE(std::holds_alternative<LoadInstruction>(descriptor.instructions[3]));
}

TEST(TestParser, IfInstruction) {
    std::string program = R""""(
                r = 1;
                b = 100;
                step = 1;
                loop:
                    r = r + step;
                    should_continue = r < b;
                    if should_continue goto loop;
                )"""";
    std::stringstream ss{program};
    auto descriptor = Parse(&ss);
    EXPECT_EQ(descriptor.memory_size, 0);
    EXPECT_EQ(descriptor.instructions.size(), 6);
    EXPECT_TRUE(std::holds_alternative<IfInstruction>(descriptor.instructions[5]));
}
