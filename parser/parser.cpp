#include "parser.h"
#include "tokenizer.h"
#include "../common/memory_primitives.h"
#include "../common/program_descriptor.h"

#include <unordered_map>
#include <sstream>
#include <istream>

template<typename T>
bool Is(const Token& token) {
    return std::holds_alternative<T>(token);
}

template<typename T>
T As(const Token& token) {
    return std::get<T>(token);
}

/**
 * using Token = std::variant<
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
 */

struct TokenPrinter {
    std::ostream& os;

    void operator()(const KeywordToken& keyword) {
        os << KeywordToString(keyword);
    }

    void operator()(const ConstantToken& constant) {
        os << constant.value;
    }

    void operator()(const TaggedSymbolToken& symbol) {
        os << '#' << symbol.value;
    }

    void operator()(const SymbolToken& symbol) {
        os << symbol.value;
    }

    void operator()(const AssignmentToken&) {
        os << ":=";
    }

    void operator()(const ThreadLocalAssignmentToken&) {
        os << '=';
    }

    void operator()(const BinOp& op) {
        os << BinOpToString(op);
    }

    void operator()(const SemicolonToken&) {
        os << ";";
    }

    void operator()(const ColonToken&) {
        os << ":";
    }

    // should never be invoked
    void operator()(const StreamEnd&) {
        assert(false);
    }
};

std::string GetStringRepr(const std::vector<Token> &tokens) {
    std::stringstream ss;
    TokenPrinter printer{ss};
    for (auto &token : tokens) {
        std::visit(printer, token);
        ss << ' ';
    }
    return ss.str();
}

AccessMode GetAccessModeByKeyword(KeywordToken keyword) {
    switch (keyword) {
        case SEQ_CST:
            return AccessMode::SEQ_CST;
        case REL_ACQ:
            return AccessMode::REL_ACQ;
        case REL:
            return AccessMode::REL;
        case ACQ:
            return AccessMode::ACQ;
        case RLX:
            return AccessMode::RLX;
        default:
            throw std::runtime_error{"Unexpected keyword instead of access mode"};
    }
}

class Parser {
public:
    explicit Parser(Tokenizer* tokenizer) : tokenizer_(tokenizer) {}

    ProgramDescriptor ParseAll() {
        ParseSharedState();
        ParseReserveSpace();
        FirstPassParsing();
        SecondPassParsing();
        return ProgramDescriptor{
                .memory_size = reserved_space_.value_or(0) + memory_name_.size(),
                .instructions = instructions_,
                .instructions_str = instructions_str_,
                .memory_name = memory_name_,
                .register_name = register_name_
        };
    }

private:
    void ParseSharedState() {
        auto token = tokenizer_->GetToken();
        if (Is<KeywordToken>(token) && As<KeywordToken>(token) == KeywordToken::SHARED_STATE) {
            tokenizer_->Next();
            if (!Is<ColonToken>(tokenizer_->GetToken())) {
                throw std::runtime_error{"Incorrect use of shared state syntax, expected colon"};
            }
            tokenizer_->Next();
            while (!Is<SemicolonToken>(tokenizer_->GetToken())) {
                auto cur_token = tokenizer_->GetToken();
                if (!Is<SymbolToken>(cur_token)) {
                    throw std::runtime_error{"Incorrect use of shared state syntax, expected symbol"};
                }
                auto symbol = As<SymbolToken>(cur_token);
                if (symbol_to_memory_.find(symbol.value) != symbol_to_memory_.end()) {
                    throw std::runtime_error{"Incorrect use of shared state syntax, found duplicate symbol"};
                }
                symbol_to_memory_[symbol.value] = memory_name_.size();
                memory_name_.push_back(std::move(symbol.value));
                tokenizer_->Next();
            }
            assert(Is<SemicolonToken>(tokenizer_->GetToken()));
            tokenizer_->Next();
            return;
        }
    }

    void ParseReserveSpace() {
        auto token = tokenizer_->GetToken();
        if (Is<KeywordToken>(token) && As<KeywordToken>(token) == KeywordToken::RESERVE_SPACE) {
            tokenizer_->Next();
            if (!Is<ColonToken>(tokenizer_->GetToken())) {
                throw std::runtime_error{"Incorrect use of reserve space syntax, expected colon"};
            }
            tokenizer_->Next();
            if (!Is<ConstantToken>(tokenizer_->GetToken())) {
                throw std::runtime_error{"Incorrect use of reserve space syntax, expected constant as a size to additionally reserve"};
            }
            reserved_space_ = As<ConstantToken>(tokenizer_->GetToken()).value;
            tokenizer_->Next();
            assert(Is<SemicolonToken>(tokenizer_->GetToken()));
            tokenizer_->Next();
            return;
        }
    }

    void TokenizeInstruction(const Token& starting_token) {
        std::vector<Token> instruction = {starting_token};
        Token current_token = tokenizer_->GetToken();
        while (!tokenizer_->IsDone() && !Is<SemicolonToken>(current_token)) {
            instruction.push_back(current_token);
            current_instruction_.push_back(current_token);
            tokenizer_->Next();
            current_token = tokenizer_->GetToken();
        }
        if (!Is<SemicolonToken>(current_token)) {
            throw std::runtime_error{"Each instruction should end with a semicolon"};
        }
        tokenized_instructions_.push_back(std::move(instruction));
        tokenizer_->Next();
    }

    // NOLINTNEXTLINE
    void ParseSingleInstruction() {
        Token token = tokenizer_->GetToken();
        current_instruction_.push_back(token);
        tokenizer_->Next();
        if (Is<SymbolToken>(token) && Is<ColonToken>(tokenizer_->GetToken())) {
            current_instruction_.emplace_back(ColonToken{});
            tokenizer_->Next();
            if (label_to_instruction_.find(As<SymbolToken>(token).value) != label_to_instruction_.end()) {
                throw std::runtime_error{"Repeating labels are prohibited"};
            }
            label_to_instruction_[As<SymbolToken>(token).value] = tokenized_instructions_.size();
            ParseSingleInstruction();
            return;
        }
        if (Is<SymbolToken>(token) || Is<KeywordToken>(token)) {
            TokenizeInstruction(token);
        } else {
            throw std::runtime_error{"Unexpected instruction start"};
        }
    }

    /**
     * The goal is to determine which index of instruction corresponds to each label
     */
    void FirstPassParsing() {
        while(!tokenizer_->IsDone()) {
            current_instruction_.clear();
            ParseSingleInstruction();
            instructions_str_.push_back(GetStringRepr(current_instruction_));
        }
    }

    Register GetRegister(const std::string& register_name) {
        if (symbol_to_register_.find(register_name) == symbol_to_register_.end()) {
            symbol_to_register_[register_name] = register_name_.size();
            register_name_.push_back(register_name);
        }
        return symbol_to_register_[register_name];
    }

    Register GetMemory(const std::string& memory_alias) {
        assert(symbol_to_memory_.find(memory_alias) != symbol_to_memory_.end());
        return symbol_to_memory_[memory_alias];
    }

    Instruction GetInstructionFromTokens(const std::vector<Token>& tokens) {
        if (tokens.empty()) {
            throw std::runtime_error{"Can not parse an empty instruction"};
        }
        if (!Is<SymbolToken>(tokens[0]) && !Is<KeywordToken>(tokens[0])) {
            throw std::runtime_error{"Invalid tokens as a start of an instruction"};
        }
        if (Is<SymbolToken>(tokens[0])) {
            Register dst = GetRegister(As<SymbolToken>(tokens[0]).value);
            if (Is<ThreadLocalAssignmentToken>(tokens[1])) {
                if (tokens.size() == 3) { // expect either constant or symbol corresponding to some global variable on the right hand side
                    if (Is<SymbolToken>(tokens[2])) {
                        return RegisterConstantAssignment{.dst = dst, .value = GetMemory(As<SymbolToken>(tokens[2]).value)};
                    } else if (Is<ConstantToken>(tokens[2])) {
                        return RegisterConstantAssignment{.dst = dst, .value = As<ConstantToken>(tokens[2]).value};
                    } else {
                        throw std::runtime_error{"Thread local assignment with unknown token"};
                    }
                } else {
                    assert(tokens.size() == 5); // r = r1 op r2
                    return RegisterBinOpAssignment{
                            GetRegister(As<SymbolToken>(tokens[0]).value),
                            GetRegister(As<SymbolToken>(tokens[2]).value),
                            GetRegister(As<SymbolToken>(tokens[4]).value),
                            As<BinOp>(tokens[3])
                    };
                }
            } else if (Is<AssignmentToken>(tokens[1])) {
                assert(Is<KeywordToken>(tokens[2]));
                switch (As<KeywordToken>(tokens[2])) {
                    case FAI: { // r1 := fai m #r2 r3
                        if(!(Is<KeywordToken>(tokens[3]) && Is<TaggedSymbolToken>(tokens[4]) && Is<SymbolToken>(tokens[5]))) {
                            throw std::runtime_error{"Incorrect usage of fetch-and-increment instruction"};
                        }
                        return FaiInstruction{
                                GetAccessModeByKeyword(As<KeywordToken>(tokens[3])),
                                dst,
                                GetRegister(As<TaggedSymbolToken>(tokens[4]).value),
                                GetRegister(As<SymbolToken>(tokens[5]).value)
                        };
                    }
                    case CAS: { // r1 := cas m #r2 r3 r4
                        if(!(Is<KeywordToken>(tokens[3]) && Is<TaggedSymbolToken>(tokens[4]) && Is<SymbolToken>(tokens[5]) && Is<SymbolToken>(tokens[6]))) {
                            throw std::runtime_error{"Incorrect usage of compare-and-swap instruction"};
                        }
                        return CasInstruction{
                                GetAccessModeByKeyword(As<KeywordToken>(tokens[3])),
                                dst,
                                GetRegister(As<TaggedSymbolToken>(tokens[4]).value),
                                GetRegister(As<SymbolToken>(tokens[5]).value),
                                GetRegister(As<SymbolToken>(tokens[6]).value)
                        };
                    }
                    default:
                        throw std::runtime_error{"Unexpected keyword in assignment"};
                }
            } else {
                throw std::runtime_error{"Expected assignment token"};
            }
        } else {
            auto keyword = As<KeywordToken>(tokens[0]);
            switch (keyword) {
                case LOAD: // load m #r1 r2
                    if (tokens.size() != 4) {
                        throw std::runtime_error{"Incorrect load instruction, wrong number of arguments"};
                    }
                    if (!(Is<KeywordToken>(tokens[1]) && Is<TaggedSymbolToken>(tokens[2]) && Is<SymbolToken>(tokens[3]))) {
                        throw std::runtime_error{"Incorrect type of arguments of load instruction"};
                    }
                    return LoadInstruction{
                            GetAccessModeByKeyword(As<KeywordToken>(tokens[1])),
                            GetRegister(As<SymbolToken>(tokens[3]).value),
                            GetRegister(As<TaggedSymbolToken>(tokens[2]).value)
                    };
                case STORE: // store m #r1 r2
                    if (tokens.size() != 4) {
                        throw std::runtime_error{"Incorrect load instruction, wrong number of arguments"};
                    }
                    if (!(Is<KeywordToken>(tokens[1]) && Is<TaggedSymbolToken>(tokens[2]) && Is<SymbolToken>(tokens[3]))) {
                        throw std::runtime_error{"Incorrect type of arguments of load instruction"};
                    }
                    return StoreInstruction{
                            GetAccessModeByKeyword(As<KeywordToken>(tokens[1])),
                            GetRegister(As<TaggedSymbolToken>(tokens[2]).value),
                            GetRegister(As<SymbolToken>(tokens[3]).value)
                    };
                case IF: // if r goto L
                    if (tokens.size() != 4) {
                        throw std::runtime_error{"Incorrect load instruction, wrong number of arguments"};
                    }
                    if (!(Is<SymbolToken>(tokens[1]) && Is<KeywordToken>(tokens[2]) && As<KeywordToken>(tokens[2]) == KeywordToken::GOTO && Is<SymbolToken>(tokens[3]))) {
                        throw std::runtime_error{"Incorrect type of arguments of load instruction"};
                    }
                    if (label_to_instruction_.find(As<SymbolToken>(tokens[3]).value) == label_to_instruction_.end()) {
                        throw std::runtime_error{"Unknown label in conditional jump instruction"};
                    }
                    return IfInstruction{
                            GetRegister(As<SymbolToken>(tokens[1]).value),
                            label_to_instruction_[As<SymbolToken>(tokens[3]).value]
                    };
                case FENCE: // fence m
                    if (tokens.size() != 2) {
                        throw std::runtime_error{"Incorrect fence instruction, wrong number of arguments"};
                    }
                    if (!Is<KeywordToken>(tokens[1])) {
                        throw std::runtime_error{"Found unexpected token at access mode parameter of fence instruction"};
                    }
                    return FenceInstruction{
                            GetAccessModeByKeyword(As<KeywordToken>(tokens[1]))
                    };
                default:
                    throw std::runtime_error{"Unexpected keyword in the beginning of instruction"};
            }
        }
    }

    /**
     * The goal is to get final instructions set
     */
    void SecondPassParsing() {
        for (const auto & tokenized_instruction : tokenized_instructions_) {
            instructions_.push_back(GetInstructionFromTokens(tokenized_instruction));
        }
    }



    std::vector<Token> current_instruction_;
    std::optional<size_t> reserved_space_;
    std::vector<std::vector<Token>> tokenized_instructions_;
    Tokenizer* tokenizer_;
    std::vector<Instruction> instructions_;
    std::unordered_map<std::string, size_t> label_to_instruction_;
    std::vector<std::string> instructions_str_;
    std::unordered_map<std::string, MemoryCell> symbol_to_memory_;
    std::vector<std::string> memory_name_;
    std::unordered_map<std::string, Register> symbol_to_register_;
    std::vector<std::string> register_name_;
};


ProgramDescriptor Parse(std::istream* is) {
    Tokenizer tokenizer(is);
    Parser parser(&tokenizer);
    return parser.ParseAll();
}