#include <gtest/gtest.h>

#include <sstream>
#include <variant>
#include <string>

#include "../parser/tokenizer.h"

TEST(TestTokenizer, Semicolon) {
    std::string data = ";";
    std::stringstream ss{data};
    Tokenizer tokenizer{&ss};
    ASSERT_TRUE(std::holds_alternative<SemicolonToken>(tokenizer.GetToken()));
    tokenizer.Next();
    ASSERT_TRUE(tokenizer.IsDone());
}

TEST(TestTokenizer, Colon) {
    std::string data = ":";
    std::stringstream ss{data};
    Tokenizer tokenizer{&ss};
    ASSERT_TRUE(std::holds_alternative<ColonToken>(tokenizer.GetToken()));
    tokenizer.Next();
    ASSERT_TRUE(tokenizer.IsDone());
}

TEST(TestTokenizer, Symbol) {
    std::string data = "abc";
    std::stringstream ss{data};
    Tokenizer tokenizer{&ss};
    ASSERT_TRUE(std::holds_alternative<SymbolToken>(tokenizer.GetToken()));
    ASSERT_EQ(std::get<SymbolToken>(tokenizer.GetToken()).value, "abc");
    tokenizer.Next();
    ASSERT_TRUE(tokenizer.IsDone());
}

TEST(TestTokenizer, TaggedSymbol) {
    std::string data = "#abc";
    std::stringstream ss{data};
    Tokenizer tokenizer{&ss};
    ASSERT_TRUE(std::holds_alternative<TaggedSymbolToken>(tokenizer.GetToken()));
    ASSERT_EQ(std::get<TaggedSymbolToken>(tokenizer.GetToken()).value, "abc");
    tokenizer.Next();
    ASSERT_TRUE(tokenizer.IsDone());
}

TEST(TestTokenizer, ThreadLocalAssignmentToken) {
    std::string data = "=";
    std::stringstream ss{data};
    Tokenizer tokenizer{&ss};
    ASSERT_TRUE(std::holds_alternative<ThreadLocalAssignmentToken>(tokenizer.GetToken()));
    tokenizer.Next();
    ASSERT_TRUE(tokenizer.IsDone());
}

TEST(TestTokenizer, AssignmentToken) {
    std::string data = ":=";
    std::stringstream ss{data};
    Tokenizer tokenizer{&ss};
    ASSERT_TRUE(std::holds_alternative<AssignmentToken>(tokenizer.GetToken()));
    tokenizer.Next();
    ASSERT_TRUE(tokenizer.IsDone());
}

TEST(TestTokenizer, BinOp) {
    std::string data = "+";
    std::stringstream ss{data};
    Tokenizer tokenizer{&ss};
    ASSERT_TRUE(std::holds_alternative<BinOp>(tokenizer.GetToken()));
    ASSERT_EQ(std::get<BinOp>(tokenizer.GetToken()), BinOp::ADD);
    tokenizer.Next();
    ASSERT_TRUE(tokenizer.IsDone());
}

TEST(TestTokenizer, ConstantToken) {
    std::string data = "42";
    std::stringstream ss{data};
    Tokenizer tokenizer{&ss};
    ASSERT_TRUE(std::holds_alternative<ConstantToken>(tokenizer.GetToken()));
    ASSERT_EQ(std::get<ConstantToken>(tokenizer.GetToken()).value, 42);
    tokenizer.Next();
    ASSERT_TRUE(tokenizer.IsDone());
}

namespace {

    template <class T>
    void RequireEquals(const Token& token, const T& value) {
        if (const auto* p = std::get_if<T>(&token)) {
            ASSERT_TRUE(*p == value);
        } else {
            GTEST_FATAL_FAILURE_("Wrong token type");
        }
    }

    template <typename... TokenTypes>
    void CheckTokens(const std::string& str, const TokenTypes&... tokens) {
        std::stringstream ss{str};
        Tokenizer tokenizer{&ss};
        [[maybe_unused]] auto check_token = [&tokenizer](const auto& token) {
            ASSERT_FALSE(tokenizer.IsDone());
            RequireEquals(tokenizer.GetToken(), token);
            tokenizer.Next();
        };
        (check_token(tokens), ...);
        ASSERT_TRUE(tokenizer.IsDone());
    }

}  // namespace

TEST(TestTokenizer, RegisterToRegisterAssignment) {
    CheckTokens("r1 = r2", SymbolToken{"r1"}, ThreadLocalAssignmentToken{}, SymbolToken{"r2"});
}

TEST(TestTokenizer, RegisterToConstantAssignement) {
    CheckTokens("r = 1", SymbolToken{"r"}, ThreadLocalAssignmentToken{}, ConstantToken{1});
}

TEST(TestTokenizer, RegisterToBinOpAssignment) {
    CheckTokens("r1 = r2 * r3",
                SymbolToken{"r1"},
                ThreadLocalAssignmentToken{},
                SymbolToken{"r2"},
                BinOp::MULTIPLY,
                SymbolToken{"r3"}
    );
}

TEST(TestTokenizer, IfStatement) {
    CheckTokens("if cond goto main_loop",
                KeywordToken::IF,
                SymbolToken{"cond"},
                KeywordToken::GOTO,
                SymbolToken{"main_loop"}
    );
}

TEST(TestTokenizer, TestCASInstruction) {
    CheckTokens("r1 := cas SEQ_CST #r2 r3 r4",
                SymbolToken{"r1"},
                AssignmentToken{},
                KeywordToken::CAS,
                KeywordToken::SEQ_CST,
                TaggedSymbolToken{"r2"},
                SymbolToken{"r3"},
                SymbolToken{"r4"});
}

TEST(TestTokenizer, TestFAIInstruction) {
    CheckTokens("r1 := fai REL_ACQ #r2 r3",
                SymbolToken{"r1"},
                AssignmentToken{},
                KeywordToken::FAI,
                KeywordToken::REL_ACQ,
                TaggedSymbolToken{"r2"},
                SymbolToken{"r3"});
}

TEST(TestTokenizer, LabelledInstruction) {
    CheckTokens("main_loop: r1 := fai REL_ACQ #r2 r3",
                SymbolToken{"main_loop"},
                ColonToken{},
                SymbolToken{"r1"},
                AssignmentToken{},
                KeywordToken::FAI,
                KeywordToken::REL_ACQ,
                TaggedSymbolToken{"r2"},
                SymbolToken{"r3"});
}

TEST(TestTokenizer, MultilineLoopCode) {
    CheckTokens(
            R""""(
                main_loop:
                    r1 := fai REL_ACQ #r2 r3;
                    should_continue = r1 <= r4;
                    if should_continue goto main_loop;
                )"""",
            SymbolToken{"main_loop"},
            ColonToken{},
            SymbolToken{"r1"},
            AssignmentToken{},
            KeywordToken::FAI,
            KeywordToken::REL_ACQ,
            TaggedSymbolToken{"r2"},
            SymbolToken{"r3"},
            SemicolonToken{},
            SymbolToken{"should_continue"},
            ThreadLocalAssignmentToken{},
            SymbolToken{"r1"},
            BinOp::LESS_EQUAL,
            SymbolToken{"r4"},
            SemicolonToken{},
            KeywordToken::IF,
            SymbolToken{"should_continue"},
            KeywordToken::GOTO,
            SymbolToken{"main_loop"},
            SemicolonToken{});
}

TEST(TestTokenizer, SharedStateSyntax) {
    CheckTokens("shared_state: x y z;",
            KeywordToken::SHARED_STATE,
            ColonToken{},
            SymbolToken{"x"},
            SymbolToken{"y"},
            SymbolToken{"z"},
            SemicolonToken{});
}

TEST(TestTokenizer, MemoryAccessMode) {
    CheckTokens("SEQ_CST REL ACQ REL_ACQ RLX",
                KeywordToken::SEQ_CST,
                KeywordToken::REL,
                KeywordToken::ACQ,
                KeywordToken::REL_ACQ,
                KeywordToken::RLX);
}
