#include "tokenizer.h"

#include <map>

Tokenizer::Tokenizer(std::istream* is) : is_(is) {
    Next();
}

void Tokenizer::Next() {
    *is_ >> std::ws;
    int c = is_->peek();
    if (c == EOF) {
        current_token_ = StreamEnd{};
        return;
    }
    if (IsStartOfNumber(c)) {
        current_token_ = ConstantToken{ ReadNumber() };
        return;
    }
    if (IsStartOfSymbol(c)) {
        current_token_ = SymbolToken { ReadSymbol() };
        TryToConvertSymbolToKeyword();
        return;
    }
    if (c == '#') {
        is_->get();
        if (!IsStartOfSymbol(is_->peek())) {
            throw std::runtime_error{"Tag is met, but expected a symbol right after"};
        }
        current_token_ = TaggedSymbolToken{ ReadSymbol() };
        return;
    }
    if (IsBinOp(c)) {
        std::string bin_op;
        bin_op += static_cast<char>(is_->get());
        if (is_->peek() == '=') {
            bin_op += static_cast<char>(is_->get());
        }
        current_token_ = BinOp{GetBinOpToken(bin_op) };
        return;
    }
    if (c == '=') {
        current_token_ = ThreadLocalAssignmentToken{};
        is_->get();
        return;
    }
    if (c == ':') {
        is_->get();
        c = is_->peek();
        if (c == '=') {
            current_token_ = AssignmentToken{};
            is_->get();
        } else {
            current_token_ = ColonToken{};
        }
        return;
    }
    if (c == ';') {
        is_->get();
        current_token_ = SemicolonToken{};
        return;
    }
    throw std::runtime_error{"Faced unknown symbol when tokenizing input"};
}

Token Tokenizer::GetToken() const {
    return current_token_;
}

bool Tokenizer::IsDone() const {
    return std::holds_alternative<StreamEnd>(current_token_);
}

bool Tokenizer::IsStartOfNumber(int c) {
    return std::isdigit(c);
}

bool Tokenizer::IsStartOfSymbol(int c) {
    return std::isalpha(c);
}

bool Tokenizer::IsInternalOfSymbol(int c) {
    return std::isalnum(c) || c == '_';
}

bool Tokenizer::IsBinOp(int c) {
    switch (c) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '>':
        case '<':
            return true;
        default:
            return false;
    }
}

bool Tokenizer::IsStartOfTaggedSymbol(int c) {
    return c == '#';
}

void Tokenizer::TryToConvertSymbolToKeyword() {
    static std::map<std::string, KeywordToken> string_to_token = {
            {"goto", KeywordToken::GOTO},
            {"if", KeywordToken::IF},
            {"cas", KeywordToken::CAS},
            {"fai", KeywordToken::FAI},
            {"load", KeywordToken::LOAD},
            {"store", KeywordToken::STORE},
            {"fence", KeywordToken::FENCE},
            {"shared_state", KeywordToken::SHARED_STATE},
            {"reserve_space", KeywordToken::RESERVE_SPACE},
            {"SEQ_CST", KeywordToken::SEQ_CST},
            {"REL", KeywordToken::REL},
            {"ACQ", KeywordToken::ACQ},
            {"REL_ACQ", KeywordToken::REL_ACQ},
            {"RLX", KeywordToken::RLX},
    };
    std::string& val = std::get<SymbolToken>(current_token_).value;
    if (string_to_token.find(val) != string_to_token.end()) {
        this->current_token_ = string_to_token[val];
    }
}

std::string KeywordToString(KeywordToken keyword) {
    switch (keyword) {
        case CAS:
            return "cas";
        case FAI:
            return "fai";
        case GOTO:
            return "goto";
        case IF:
            return "if";
        case SHARED_STATE:
            return "shared_state";
        case RESERVE_SPACE:
            return "reserve_space";
        case LOAD:
            return "load";
        case STORE:
            return "store";
        case SEQ_CST:
            return "SEQ_CST";
        case REL_ACQ:
            return "REL_ACQ";
        case REL:
            return "REL";
        case ACQ:
            return "ACQ";
        case RLX:
            return "RLX";
        case FENCE:
            return "fence";
        default:
            assert(false);
    }
}

std::string BinOpToString(BinOp op) {
    switch (op) {
        case ADD:
            return "+";
        case SUBTRACT:
            return "-";
        case DIVIDE:
            return "/";
        case MULTIPLY:
            return "*";
        case LESS:
            return "<";
        case GREATER:
            return ">";
        case LESS_EQUAL:
            return "<=";
        case GREATER_EQUAL:
            return ">=";
        default:
            assert(false);
    }
}

ConstantToken::Type Tokenizer::ReadNumber() {
    ConstantToken::Type num;
    *is_ >> num;
    return num;
}

BinOp Tokenizer::GetBinOpToken(std::string const& s) {
    assert(!s.empty());
    if (s.size() > 2) {
        throw std::runtime_error{"Binary operation token of unexpected size"};
    }
    if (s.size() == 1) {
        switch (s[0]) {
            case '+':
                return BinOp::ADD;
            case '-':
                return BinOp::SUBTRACT;
            case '*':
                return BinOp::MULTIPLY;
            case '/':
                return BinOp::DIVIDE;
            case '<':
                return BinOp::LESS;
            case '>':
                return BinOp::GREATER;
            default:
                throw std::runtime_error{"Unknown binary operation token of length 1"};
        }
    }

    if (s == "<=") {
        return BinOp::LESS_EQUAL;
    }
    if (s == ">=") {
        return BinOp::GREATER_EQUAL;
    }

    throw std::runtime_error{"Unknown binary operation token of length 2"};
}

std::string Tokenizer::ReadSymbol() {
    int c = is_->peek();
    std::string value;
    while (IsInternalOfSymbol(c)) {
        value += static_cast<char>(is_->get());
        c = is_->peek();
    }
    return value;
}

bool ThreadLocalAssignmentToken::operator==(const ThreadLocalAssignmentToken&) const {
    return true;
}

bool AssignmentToken::operator==(const AssignmentToken&) const {
    return true;
}

bool SymbolToken::operator==(const SymbolToken& other) const {
    return value == other.value;
}

bool TaggedSymbolToken::operator==(const TaggedSymbolToken& other) const {
    return value == other.value;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

bool StreamEnd::operator==(const StreamEnd&) const {
    return true;
}

bool SemicolonToken::operator==(const SemicolonToken&) const {
    return true;
}

bool ColonToken::operator==(const ColonToken&) const {
    return true;
}
