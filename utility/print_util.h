#ifndef PRINT_UTIL_H
#define PRINT_UTIL_H
#include <ostream>

struct Indent {
    size_t val = 0;
};

inline std::ostream& operator<<(std::ostream& os, const Indent& indent) {
    for (size_t i = 0; i < indent.val; ++i) {
        os << '\t';
    }
    return os;
}

#endif //PRINT_UTIL_H
