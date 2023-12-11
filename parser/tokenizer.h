#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <istream>
#include <variant>
#include <string>
#include <cctype>
#include <cassert>
#include <exception>

enum BinOp {
    ADD, SUBTRACT, DIVIDE, MULTIPLY, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL
};

enum KeywordToken {
    CAS, FAI, GOTO, IF, SHARED_STATE, LOAD, STORE, SEQ_CST, REL_ACQ, REL, ACQ, RLX, FENCE, RESERVE_SPACE
};

struct ThreadLocalAssignmentToken {
    bool operator==(const ThreadLocalAssignmentToken&) const;
};

struct AssignmentToken {
    bool operator==(const AssignmentToken&) const;
};

struct SymbolToken {
    std::string value;
    bool operator==(const SymbolToken&) const;
};

// store tokens for which we have '#' right in front of them
// (for instance, "#foo" will be stored here with value = "foo")
struct TaggedSymbolToken {
    std::string value;
    bool operator==(const TaggedSymbolToken&) const;
};

struct ConstantToken{
    using Type = uint64_t;
    Type value;
    bool operator==(const ConstantToken&) const;
};

struct StreamEnd {
    bool operator==(const StreamEnd&) const;
};

struct SemicolonToken {
    bool operator==(const SemicolonToken&) const;
};

struct ColonToken {
    bool operator==(const ColonToken&) const;
};

using Token = std::variant<
        ConstantToken,
        TaggedSymbolToken,
        SymbolToken,
        AssignmentToken,
        ThreadLocalAssignmentToken,
        BinOp,
        SemicolonToken,
        ColonToken,
        StreamEnd,
        KeywordToken>;

std::string KeywordToString(KeywordToken keyword);

std::string BinOpToString(BinOp op);

class Tokenizer{
public:
    explicit Tokenizer(std::istream* is);

    void Next();

    [[nodiscard]] Token GetToken() const;

    [[nodiscard]] bool IsDone() const;

private:

    static bool IsStartOfNumber(int c);

    static bool IsStartOfSymbol(int c);

    static bool IsInternalOfSymbol(int c);

    static bool IsBinOp(int c);

    static bool IsStartOfTaggedSymbol(int c);

    ConstantToken::Type ReadNumber();

    static BinOp GetBinOpToken(std::string const& s);

    void TryToConvertSymbolToKeyword();

    std::string ReadSymbol();

    std::istream* is_;
    Token current_token_;
};
#endif //TOKENIZER_H